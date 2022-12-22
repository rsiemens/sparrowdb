#include <stdbool.h>
#include <stdio.h>

#define assert(cond) \
    do { \
        if (!(cond)) { \
            printf("    Assertion failed: (\"%s\"), function \"%s\", file \"%s\", line %d\n", #cond, __FUNCTION__, __FILE__, __LINE__); \
            return; \
        } \
    } while(0);

#define TEST_START() \
    printf("  ...%s\n", __FUNCTION__);

#define TEST_GROUP(name) \
    printf("%s\n", name);
