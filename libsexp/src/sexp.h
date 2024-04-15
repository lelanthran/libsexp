
#ifndef H_SEXP
#define H_SEXP

typedef struct sexp_t sexp_t;

#ifdef __cplusplus
extern "C" {
#endif

   sexp_t sexp_new (const char *fname, int line, int cpos,
                    const char **text);

   void sexp_del (sexp_t *sexp);


#ifdef __cplusplus
};
#endif


#endif


