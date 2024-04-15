#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "ds_str.h"
#include "ds_hmap.h"
#include "ds_array.h"

#include "sexp.h"

struct sexp_t {
   char *fname;
   int line;
   int cpos;

   enum sexp_type_t type;
   char *text;
   size_t textlen;

   ds_hmap_t *attrs;       // key: value
   ds_array_t *children;   // ordered children
   sexp_t *parent;         // Parent
};




#define TOKEN_BREAK     (-1)

static void update_position (int c, int *line, int *cpos)
{
   *cpos = (*cpos) + 1;
   if (c == '\n') {
      *line = (*line) + 1;
      *cpos = 0;
   }
}


static enum sexp_err_t read_string (char **dst, int delim, char **input,
                                    int *line, int *cpos)
{
   enum sexp_err_t error = sexp_err_NO_MEM;

   char *tmp = *input;
   char *text = NULL;

   int c;
   while ((c = *tmp++) && (c != delim)) {
      char str[2] = { (char)c, 0 };

      if (delim == TOKEN_BREAK) {
         static const char *breakchars = "() \t\n\r:=\"";
         if ((strchr (breakchars, c))) {
            error = sexp_err_OK;
            tmp--;
            break;
         }
      }

      update_position (c, line, cpos);

      if (c == '\\') {
         if ((c = *tmp++) == 0)
            break;
         str[0] = (char)c;
         if (!(ds_str_append (&text, str, NULL)))
            goto cleanup;
      } else {
         if (!(ds_str_append (&text, str, NULL)))
            goto cleanup;
      }
   }

   if (c == 0) {
      error = sexp_err_UNTERMINATED_STRING;
      goto cleanup;
   }

   if (c == delim) {
      error = sexp_err_OK;
   }

cleanup:
   if (error)
      free (text);
   else
      *dst = text;

   *input = tmp;

   return error;
}


static enum sexp_err_t read_attrs (sexp_t *parent, char **input,
                                   int *line, int *cpos)
{
   enum sexp_err_t error = sexp_err_NO_MEM;
   char *key = NULL;
   char *value = NULL;

   if ((error = read_string (&key, '=', input, line, cpos)) != 0)
      goto cleanup;

   if ((error = read_string (&value, TOKEN_BREAK, input, line, cpos)) != 0)
      goto cleanup;

   if (!key) {
      error = sexp_err_UNRECOGNISED_INPUT;
      goto cleanup;
   }

   if (!value) {
      if (!(value = ds_str_dup (""))) {
         goto cleanup;
      }
   }

   if (parent) {
      if (!(ds_hmap_set_str_str (parent->attrs, key, value)))
         goto cleanup;
   } else {
      free (value);
   }

   free (key);

   error = sexp_err_OK;

cleanup:
   if (error) {
      free (key);
      free (value);
   }
   return error;
}


static enum sexp_err_t read_list (sexp_t *parent, char **input,
                                  const char *fname, int *line, int *cpos)
{
   sexp_t *sexp;
   enum sexp_err_t error;

   while ((error = sexp_next (&sexp, parent, fname, line, cpos, input)) == 0) {
      ;
   }
   return error == sexp_err_LIST_TERMINATOR ? 0 : error;
}


static enum sexp_err_t read_number (char **dst, enum sexp_type_t *type, char **input,
                                    int *line, int *cpos)
{
   enum sexp_err_t error = sexp_err_NO_MEM;
   char *tmp = *input;
   char *text = NULL;
   char str[2] = { 0, 0 };

   size_t ndots = 0;
   char c;

   while ((c = *tmp++) && (isdigit (c))) {
      update_position (c, line, cpos);
      if (c == '.')
         ndots++;
      if (ndots > 1) {
         error = sexp_err_MANGLED_NUMBER;
         goto cleanup;
      }

      str[0] = c;
      if (!(ds_str_append (&text, str, NULL)))
         goto cleanup;
   }

   *type = ndots > 0 ? sexp_FLOAT : sexp_INTEGER;

   error = sexp_err_OK;
cleanup:
   if (error) {
      free (text);
      text = NULL;
   }
   *dst = text;
   *input = tmp;
   return error;
}


static enum sexp_err_t read_symbol (char **dst, char **input, int *line, int *cpos)
{
   return read_string (dst, TOKEN_BREAK, input, line, cpos);
}


static enum sexp_err_t read_comment (char **dst, char **input, int *line, int *cpos)
{
   return read_string (dst, '\n', input, line, cpos);
}


/* **************************************************************************** */

enum sexp_err_t sexp_next (sexp_t **dst, sexp_t *parent,
                           const char *fname, int *line, int *cpos,
                           char **input)
{
   enum sexp_err_t error = sexp_err_NO_MEM;
   size_t parent_index = (size_t)-1;
   sexp_t *ret = calloc (1, sizeof *ret);
   char *text = NULL;

   if (!ret)
      goto cleanup;

   if (!(ret->fname = ds_str_dup (fname)))
      goto cleanup;

   ret->line = *line;
   ret->cpos = *cpos;

   if (!(ret->attrs = ds_hmap_new (256)))
      goto cleanup;

   if (!(ret->children = ds_array_new ()))
      goto cleanup;

   if (parent) {
      ret->parent = parent;
      parent_index = ds_array_length (parent->children);
      if (!(ds_array_ins_tail (parent->children, ret))) {
         parent_index = (size_t)-1;
         goto cleanup;
      }
   }

   // Skip the leading whitespace
   int c;
   while ((c = (*input)[0]) && isspace (c)) {
      *input = (*input) + 1;
      update_position (c, line, cpos);
   }

   // Handle each possible type
   if (c == 0) {
      sexp_del (ret);
      ret = NULL;
      if (parent_index != (size_t)-1) {
         ds_array_rm (parent->children, parent_index);
      }
      error = sexp_err_EOF;
      goto cleanup;
   }

   if (c == '"') {
      *input = (*input) + 1;
      error = read_string (&text, '"', input, line, cpos);
      ret->type = sexp_STRING;
      goto cleanup;
   }

   if (c == ';') {
      *input = (*input) + 1;
      error = read_comment (&text, input, line, cpos);
      ret->type = sexp_COMMENT;
      goto cleanup;
   }

   if (c == ':') {
      *input = (*input) + 1;
      error = read_attrs (parent, input, line, cpos);
      sexp_del (ret);
      ret = NULL;
      if (parent_index != (size_t)-1) {
         ds_array_rm (parent->children, parent_index);
      }

      error = sexp_err_OK;
      goto cleanup;
   }

   if (c == '(') {
      *input = (*input) + 1;
      error = read_list (ret, input, fname, line, cpos);
      ret->type = sexp_LIST;
      goto cleanup;
   }

   if (c == ')') {
      *input = (*input) + 1;
      sexp_del (ret);
      ret = NULL;
      if (parent_index != (size_t)-1) {
         ds_array_rm (parent->children, parent_index);
      }
      error = sexp_err_LIST_TERMINATOR;
      goto cleanup;
   }

   if (isdigit (c)) {
      error = read_number (&text, &ret->type, input, line, cpos);
      goto cleanup;
   }

   if ((isalpha (c))) {
      error = read_symbol (&text, input, line, cpos);
      ret->type = sexp_SYMBOL;
      goto cleanup;
   }

   error = sexp_err_UNRECOGNISED_INPUT;

cleanup:

   if (!error && ret && text) {
      ret->text = text;
      ret->textlen = strlen (text);
   }

   if (error) {
      if (parent_index != (size_t)-1) {
         ds_array_rm (parent->children, parent_index);
      }

      sexp_del (ret);
      ret = NULL;
   }
   *dst = ret;
   return error;
}




#if 0    // Don't worry about parent, parent must delete child itself
static size_t sexp_find_child_index (sexp_t *sexp, sexp_t *child)
{
   size_t nchildren = ds_array_length (sexp->children);
   for (size_t i=0; i<nchildren; i++) {
      sexp_t *tmp = ds_array_get (sexp->children, i);
      if (tmp == child) {
         return i;
      }
   }
   return (size_t)-1;
}
#endif

void sexp_del (sexp_t *sexp)
{
   if (!sexp) {
      return;
   }

#if 0    // Don't worry about parent, parent must delete child itself
   if (sexp->parent) {
      size_t my_index = sexp_find_child_index (sexp->parent, sexp);
      if (my_index != (size_t)-1) {
         ds_array_rm (sexp->parent->children, my_index);
      }
   }
#endif

   free (sexp->fname);
   free (sexp->text);

   ds_hmap_fptr (sexp->attrs, free);
   ds_hmap_del (sexp->attrs);

   ds_array_fptr (sexp->children, (void (*) (void *))sexp_del);
   ds_array_del (sexp->children);

   free (sexp);
}

bool sexp_get_attrs (sexp_t *sexp, char ***keys, char ***values)
{
   bool error = true;
   char **lkeys = NULL;

   if (!sexp) {
      return NULL;
   }

   size_t nattrs = ds_hmap_keys (sexp->attrs, (void ***)&lkeys, NULL);

  if (!lkeys) {
      return NULL;
   }

   if (keys) {
      if (!(*keys = calloc (nattrs + 1, sizeof **keys))) {
         goto cleanup;
      }

   }
   if (values) {
      if (!(*values = calloc (nattrs + 1, sizeof **values))) {
         if (keys) {
            free (*keys);
            *keys = NULL;
         }
         goto cleanup;
      }
   }

   for (size_t i=0; lkeys[i]; i++) {
      if (keys) {
         (*keys)[i] = lkeys[i];
      }
      if (values) {
         char *tmp = NULL;
         if (!(ds_hmap_get_str_str (sexp->attrs, lkeys[i], &tmp))) {
            if (keys) {
               free (*keys);
               *keys = NULL;
            }
            free (*values);
            goto cleanup;
         }
         (*values)[i] = tmp;
      }
   }

   error = false;

cleanup:
   free (lkeys);
   return !error;
}

const char *sexp_type_name (enum sexp_type_t type)
{
   static char buffer[55];
   switch (type) {
      case sexp_INTEGER:   return "INTEGER";
      case sexp_FLOAT:     return "FLOAT";
      case sexp_STRING:    return "STRING";
      case sexp_SYMBOL:    return "SYMBOL";
      case sexp_LIST:      return "LIST";
      case sexp_COMMENT:   return "COMMENT";
      case sexp_UNKNOWN:
      default:
         snprintf (buffer, sizeof buffer, "Unknown sexp type found %i", type);
         return buffer;
         break;
   }
}

const char *sexp_error_name (enum sexp_err_t error)
{
   static const struct {
      enum sexp_err_t error;
      const char *name;
   } names[] = {
#define SEXP_ERROR(x)   { x, #x }
      SEXP_ERROR (sexp_err_OK),
      SEXP_ERROR (sexp_err_EOF),
      SEXP_ERROR (sexp_err_UNKNOWN),
      SEXP_ERROR (sexp_err_NO_MEM),
      SEXP_ERROR (sexp_err_UNRECOGNISED_INPUT),
      SEXP_ERROR (sexp_err_UNTERMINATED_STRING),
      SEXP_ERROR (sexp_err_MANGLED_NUMBER),
      SEXP_ERROR (sexp_err_LIST_TERMINATOR),
#undef SEXP_ERROR
   };

   static char buffer[55];

   static const size_t nnames = sizeof names/sizeof names[0];

   for (size_t i=0; i<nnames; i++) {
      if (names[i].error == error) {
         return names[i].name;
      }
   }
   snprintf (buffer, sizeof buffer, "Unknown error: %i", error);
   return buffer;
}


void sexp_dump (sexp_t *sexp, FILE *outf, size_t level)
{
   if (!outf)
      outf = stdout;

#define INDENT    for (size_t i=0; i<(level * 3); i++) fputc (' ', outf);

   if (!sexp) {
      INDENT;
      fprintf (outf, "NULL sexp object\n");
      return;
   }

   INDENT;
   fprintf (outf, "sexp[%s]: %s\n", sexp_type_name (sexp->type), sexp->text);

   char **keys = NULL;
   char **values = NULL;

   if (!(sexp_get_attrs (sexp, &keys, &values))) {
      fprintf (outf, "Failure while retrieving attributes\n");
   }

   level += 1;
   for (size_t i=0; keys && keys[i]; i++) {
      INDENT;
      fprintf (outf, "[%s:%s]\n", keys[i], values[i]);
   }
   level -= 1;

   free (keys);
   free (values);

   size_t nchildren = ds_array_length (sexp->children);
   for (size_t i=0; i<nchildren; i++) {
      sexp_t *child = ds_array_get (sexp->children, i);
      sexp_dump (child, outf, level + 1);
   }
}

