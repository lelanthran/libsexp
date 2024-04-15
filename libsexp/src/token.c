#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ds_str.h"

#include "token.h"


/* **************************************************************** */

static char *fslurp (FILE *inf)
{
   if ((fseek (inf, 0, SEEK_END)) != 0) {
      fprintf (stderr, "Failed to set file point to end of file\n");
      return NULL;
   }

   long len = ftell (inf);
   len += 1;
   if ((fseek (inf, 0, SEEK_SET)) != 0) {
      fprintf (stderr, "Failed to set file pointer to beginning of file\n");
      return NULL;
   }

   char *ret = NULL;
   if (!(ret = calloc (1, len))) {
      fprintf (stderr, "Out of memory error trying to allocate %li bytes\n", len);
      return NULL;
   }

   size_t nbytes = fread (ret, 1, len - 1, inf);
   if (nbytes != (size_t)len) {
      fprintf (stderr, "Unexpected number of bytes read. Expected %li, got %zu\n",
            len, nbytes);
      free (ret);
      ret = NULL;
   }

   return ret;
}


/* **************************************************************** */
struct token_engine_t {
   char *fname;
   char *input;
   size_t input_len;

   size_t index;
   size_t line;
   size_t cpos;
};

token_engine_t *token_engine_new (FILE *inf, const char *fname)
{
   token_engine_t *ret = calloc (1, sizeof *ret);
   if (!ret) {
      return NULL;
   }

   if (!(ret->fname= ds_str_dup (fname))) {
      free (ret);
      return NULL;
   }

   if (!(ret->input = fslurp (inf))) {
      free (ret->fname);
      free (ret);
      return NULL;
   }

   ret->input_len = strlen (ret->input);
   ret->line = 1;
   ret->cpos = 1;
   return ret;
}

void token_engine_del (token_engine_t **te)
{
   if (!te || !*te) {
      return;
   }

   free ((*te)->fname);
   free ((*te)->input);
   free (*te);
   *te = NULL;
}



/* **************************************************************** */


enum token_type_t {
   token_type_UNKNOWN = 0,
   token_type_END,
   token_type_SYMBOL,
   token_type_STRING,
   token_type_INTEGER,
   token_type_COLON,
   token_type_EQUAL,
   token_type_HASH,
   token_type_SEMICOLON,
   token_type_OPEN,
   token_type_CLOSE,
};

static const char *token_type_name (enum token_type_t type)
{
   static const struct {
      enum token_type_t type;
      const char *name;
   } tokens[] = {
#define TOKEN(x)     { x, #x }
      TOKEN (token_type_UNKNOWN),
      TOKEN (token_type_END),
      TOKEN (token_type_SYMBOL),
      TOKEN (token_type_STRING),
      TOKEN (token_type_INTEGER),
      TOKEN (token_type_COLON),
      TOKEN (token_type_EQUAL),
      TOKEN (token_type_HASH),
      TOKEN (token_type_SEMICOLON),
      TOKEN (token_type_OPEN),
      TOKEN (token_type_CLOSE),
#undef TOKEN
   };
   static const size_t ntokens = sizeof tokens/sizeof tokens[0];
   static char unknown[55];

   for (size_t i=0; i<ntokens; i++) {
      if (tokens[i].type == type) {
         return tokens[i].name;
      }
   }

   snprintf (unknown, sizeof unknown, "Unknown token type: %i", type);
   return unknown;
}

/* **************************************************************** */

struct token_t {
   enum token_type_t type;
   char *text;
   size_t line;
   size_t cpos;
};

static void token_del (token_t **t)
{
   if (! t || !*t)
      return;
   free ((*t)->text);
   free (*t);
   *t = NULL;
}

static token_t *token_new (enum token_type_t type,
                           const char *text, size_t text_len,
                           size_t line, size_t cpos)
{
   token_t *ret = calloc (1, sizeof *ret);
   if (!ret)
      return NULL;

   ret->type = type;
   ret->line = line;
   ret->cpos = cpos;

   if (!(ret->text = calloc (1, text_len + 1))) {
      token_del (&ret);
   }
   memcpy (ret->text, text, text_len);

   return ret;
}


/* **************************************************************** */

static size_t pcpos = 0;
static int my_getc (token_engine_t *te)
{
   if (te->index == (size_t)-1) {
      te->index = 0;
   }

   if (te->index > te->input_len) {
      return EOF;
   }
   int c = te->input[te->index++];
   if (c == '\n') {
      te->line++;
      te->cpos = 0;
   } else {
      te->cpos++;
   }

   return c;
}

static void my_ungetc (token_engine_t *te)
{
   if (te->index == (size_t)-1) {
      return;
   }

   int c = te->input[--te->index];

   if (c == '\n') {
      te->cpos = 0;
      te->line--;
   } else {
      te->cpos--;
   }
}


static bool ffwd_to_next_token (token_engine_t *te)
{
   int c = EOF;

   while (((c = my_getc (te)) != EOF) && (isspace (c))) {
      ;
   }
   if ((c != EOF) && !(isspace (c))) {
      my_ungetc (te);
   }
}



/* **************************************************************** */

token_t *token_engine_next (token_engine_t *te)
{

   if (!(ffwd_to_next_token (inf, line, cpos)))
      return NULL;

   int c = my_getc (inf, line, cpos);

   switch (c) {
      case token_type_STRING:
      case token_type_COLON:
      case token_type_EQUAL:
      case token_type_HASH:
      case token_type_SEMICOLON:
      case token_type_OPEN:
      case token_type_CLOSE:

      case token_type_INTEGER:
         // Read number
         break;

      case token_type_UNKNOWN:
         // Error
         break;

      case token_type_END:
         // Done
         break;

      case token_type_SYMBOL:
      default:
         break;
   }

}

void token_free (token_t **token)
{
   token_del (token);
}

void token_dump (const token_t *token, FILE *outf)
{
   if (!outf) {
      outf = stdout;
   }
   if (!token) {
      fprintf (outf, "NULL token\n");
      return;
   }

   fprintf (outf, "Token %zu,%zu, [%s]: [%s]\n",
         token->line,
         token->cpos,
         token_type_name (token->type),
         token->text);
}


