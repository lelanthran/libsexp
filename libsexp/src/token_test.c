
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "token.h"

#define TOKENISER_INPUT    ("./tests/inputs/tokeniser_test.txt")
#define TOKENISER_OUTPUT   ("./tests/outputs/tokeniser_test.txt")

int main (void)
{
   int ret = EXIT_FAILURE;

   token_t *token = NULL;
   FILE *inf = NULL;
   size_t line = 1;
   size_t cpos = 1;

   if (!(inf = fopen (TOKENISER_INPUT, "r"))) {
      fprintf (stderr, "Failed to open [%s] for reading: %m\n",
            TOKENISER_INPUT);
      goto cleanup;
   }

   while ((token_read (inf, &token, &line, &cpos)) == true) {
      token_dump (token, stdout);
      token_free (&token);
   }

   ret = EXIT_SUCCESS;

cleanup:
   if (inf) {
      fclose (inf);
   }
   token_free (&token);
   ret = EXIT_SUCCESS;
   return ret;
}

