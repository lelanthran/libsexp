
#ifndef H_TOKENISER
#define H_TOKENISER

typedef struct token_t token_t;

#ifdef __cplusplus
extern "C" {
#endif

   bool token_read (FILE *inf, token_t **dst, size_t *line, size_t *cpos);
   void token_free (token_t **token);
   void token_dump (const token_t *token, FILE *outf);


#ifdef __cplusplus
};
#endif


#endif


