
#ifndef H_SEXP
#define H_SEXP

typedef struct sexp_t sexp_t;

enum sexp_type_t {
   sexp_UNKNOWN = 0,
   sexp_INTEGER,
   sexp_FLOAT,
   sexp_STRING,
   sexp_SYMBOL,
   sexp_LIST,
   sexp_COMMENT,
};

enum sexp_err_t {
   sexp_err_OK = 0,
   sexp_err_EOF,
   sexp_err_UNKNOWN,
   sexp_err_NO_MEM,
   sexp_err_UNRECOGNISED_INPUT,
   sexp_err_UNTERMINATED_STRING,
   sexp_err_MANGLED_NUMBER,
   sexp_err_LIST_TERMINATOR,
};


#ifdef __cplusplus
extern "C" {
#endif

   enum sexp_err_t sexp_next (sexp_t **dst, sexp_t *parent,
                              const char *fname, int *line, int *cpos,
                              char **input);

   void sexp_del (sexp_t *sexp);

   bool sexp_get_attrs (sexp_t *sexp, char ***keys, char ***values);

   const char *sexp_type_name (enum sexp_type_t type);
   const char *sexp_error_name (enum sexp_err_t error);

   void sexp_dump (sexp_t *sexp, FILE *outf, size_t level);


#ifdef __cplusplus
};
#endif


#endif


