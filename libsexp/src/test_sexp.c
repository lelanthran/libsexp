
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sexp.h"

#define TOKENISER_INPUT    ("./tests/inputs/tokeniser_test.txt")
#define TOKENISER_OUTPUT   ("./tests/outputs/tokeniser_test.txt")

int main (void)
{
   int ret = EXIT_FAILURE;

   FILE *inf = NULL;

   if (!(inf = fopen (TOKENISER_INPUT, "r"))) {
      fprintf (stderr, "Failed to open [%s] for reading: %m\n",
            TOKENISER_INPUT);
      goto cleanup;
   }

   ret = EXIT_SUCCESS;

cleanup:
   if (inf) {
      fclose (inf);
   }

   ret = EXIT_SUCCESS;
   return ret;
}

