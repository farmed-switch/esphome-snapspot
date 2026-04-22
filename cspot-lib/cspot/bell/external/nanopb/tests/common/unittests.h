#include <stdio.h>

#ifdef UNITTESTS_SHORT_MSGS

#define COMMENT(x) printf("\n----" x "----\n");
#define TEST(x) \
    if (!(x)) { \
        fprintf(stderr, "FAIL: Line %d\n", __LINE__); \
        status = 1; \
    } else { \
        printf("OK: Line %d\n", __LINE__); \
    }

#else

#define COMMENT(x) printf("\n----" x "----\n");
#define TEST(x) \
    if (!(x)) { \
        fprintf(stderr, "\033[31;1mFAILED:\033[22;39m %s:%d %s\n", __FILE__, __LINE__, #x); \
        status = 1; \
    } else { \
        printf("\033[32;1mOK:\033[22;39m %s\n", #x); \
    }
#endif

