
#ifndef H_TOKENISER
#define H_TOKENISER

typedef struct token_t token_t;
typedef struct token_engine_t token_engine_t;

#ifdef __cplusplus
extern "C" {
#endif

   token_engine_t *token_engine_new (FILE *inf, const char *fname);
   token_t *token_engine_next (token_engine_t *te);

   void token_free (token_t **token);
   void token_dump (const token_t *token, FILE *outf);


#ifdef __cplusplus
};
#endif


#endif


