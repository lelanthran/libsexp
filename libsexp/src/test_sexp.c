
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sexp.h"

#define TOKENISER_INPUT    ("./tests/inputs/tokeniser_test.txt")
#define TOKENISER_OUTPUT   ("./tests/outputs/tokeniser_test.txt")

char *fslurp (const char *fname)
{
   bool error = true;
   char *ret = NULL;
   FILE *inf = NULL;

   if (!(inf = fopen (TOKENISER_INPUT, "r"))) {
      fprintf (stderr, "Failed to open [%s] for reading: %m\n",
            TOKENISER_INPUT);
      goto cleanup;
   }

   if ((fseek (inf, 0, SEEK_END)) != 0) {
      fprintf (stderr, "Failed to seek file %s: %m\n", fname);
      goto cleanup;
   }

   long len = ftell (inf);
   if (len < 0) {
      fprintf (stderr, "Failed to get file length for %s: %m\n", fname);
      goto cleanup;
   }

   if ((fseek (inf, 0, SEEK_SET)) != 0) {
      fprintf (stderr, "Failed to reset file %s: %m\n", fname);
      goto cleanup;
   }

   if (!(ret = calloc (1, len + 1))) {
      fprintf (stderr, "Failed to allocate %li bytes for file data: %m\n", len);
      goto cleanup;
   }

   size_t nbytes = fread (ret, 1, len, inf);
   if (nbytes != (size_t)len) {
      fprintf (stderr, "Unexpected number of bytes read: expected %li, got %zu\n",
            len, nbytes);
      goto cleanup;
   }

   error = false;
cleanup:
   if (inf)
      fclose (inf);

   if (error) {
      free (ret);
      ret = NULL;
   }

   return ret;
}

int main (void)
{
   int ret = EXIT_FAILURE;

   static const char *fname = TOKENISER_INPUT;
   int line = 1, cpos = 1;

   char *input = fslurp (fname);
   char *tmp = input;
   sexp_t *sexp;

   if (!input) {
      fprintf (stderr, "Failed to read file %s, aborting\n", fname);
      goto cleanup;
   }

   enum sexp_err_t error;
   while ((error = (sexp_next (&sexp, NULL, fname, &line, &cpos, &tmp))) == sexp_err_OK) {
      sexp_dump (sexp, NULL, 0);
      if (sexp) {
         struct sexp_info_t *si = sexp_info (sexp);
         if (!si) {
            printf ("%p: Failed to get information - OOM?\n", sexp);
            *(char *)0 = 0;
         }
         sexp_info_dump (si, NULL);
         sexp_info_del (si);
      }
      sexp_del (sexp);
   }
   printf ("Last error: %s\n", sexp_error_name (error));

   ret = EXIT_SUCCESS;

cleanup:
   free (input);
   ret = EXIT_SUCCESS;
   return ret;
}

