#include "test_util.h"

int g_test_failures = 0;

/* Test declarations added by later tasks */

int main(void) {
    /* RUN_TEST calls added by later tasks */

    printf(g_test_failures ? "TESTS FAILED: %d\n" : "ALL TESTS PASSED\n", g_test_failures);
    return g_test_failures ? 1 : 0;
}
