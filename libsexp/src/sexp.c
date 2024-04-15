#include <stdlib.h>

#include "ds_str.h"

#include "sexp.h"

enum sexp_type_t {
   sexp_UNKNOWN = 0,
   sexp_INTEGER,
   sexp_FLOAT,
   sexp_STRING,
   sexp_SYMBOL,
   sexp_LIST,
};

struct sexp_t {
   char *fname;
   int line;
   int cpos;

   enum sexp_type_t type;
   char *text;
   char *textlen;
};

sexp_t sexp_new (const char *fname, int *line, int *cpos,
                 const char **text)
{
   bool error = true;
   sexp_t *ret = calloc (1, sizeof *ret);
   if (!ret) {
      goto cleanup;
   }
   if (!(ret->fname = ds_str_dup (fname)))
      goto cleanup;

   ret->line = line;
   ret->cpos = cpos;
   error = false;

cleanup:
   if (error) {
      sexp_del (ret);
      ret = NULL;
   }

   return ret;
}


void sexp_del (sexp_t *sexp);

