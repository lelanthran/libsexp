#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "ds_str.h"

#include "tokeniser.h"

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

static token_t *token_new (enum token_type_t type, const char *text,
                           size_t line, size_t cpos)
{
   token_t *ret = calloc (1, sizeof *ret);
   if (!ret)
      return NULL;

   ret->type = type;
   ret->line = line;
   ret->cpos = cpos;

   if (!(ret->text = ds_str_dup (text))) {
      token_del (&ret);
   }

   return ret;
}


/* **************************************************************** */

static size_t pcpos = 0;
static int my_getc (FILE *inf, size_t *line, size_t *cpos)
{
   int c = fgetc (inf);
   if (c == '\n') {
      *line = *line + 1;
      pcpos = *cpos;
      *cpos = 0;
      return c;
   }

   pcpos = 0;
   *cpos = *cpos + 1;
   return c;
}
static void my_ungetc (FILE *inf, int c, size_t *line, size_t *cpos)
{
   if (c == '\n') {
      *cpos = pcpos;
      pcpos = 0;
      *line = *line - 1;
   } else {
      *cpos = *cpos - 1;
   }
   ungetc (c, inf);
}


static bool find_token_start (FILE *inf, size_t *line, size_t *cpos)
{
   static const *delimiters =
      "() \r\t\n"

   int c = EOF;

   while ((c = my_getc (inf, line, cpos)) != EOF) {
      if (strchr (delimiters, c)) {
         my_ungetc (inf, c, line, cpos)
      }
   }
   return feof (inf) || ferror (inf);
}

/* **************************************************************** */

bool token_read (FILE *inf, token_t **dst, size_t *line, size_t *cpos)
{
   token_del (dst);
   if (!(find_token_start (inf, line, cpos)))
      return false;
}

void token_free (token_t **token)
{
   token_del (token);
}

void token_dump (const token_t *token, FILE *outf)
{

}


