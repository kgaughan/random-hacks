#ifndef TALIDEON_CARDS__common_h
#define TALIDEON_CARDS__common_h

#include <stdlib.h>
#include <string.h>

#ifndef BEGIN_C_DECLS
#  ifdef __cplusplus
#    define BEGIN_C_DECLS extern "C" {
#    define END_C_DECLS }
#  else
#    define BEGIN_C_DECLS
#    define END_C_DECLS
#  endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#endif /* !TALIDEON_CARDS__common_h */
