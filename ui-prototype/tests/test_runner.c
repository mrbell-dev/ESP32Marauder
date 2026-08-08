#include "test_util.h"

int g_test_failures = 0;

/* Test declarations */
void test_theme_default(void);
void test_theme_parse_ok(void);
void test_theme_parse_bad(void);

int main(void) {
    RUN_TEST(test_theme_default);
    RUN_TEST(test_theme_parse_ok);
    RUN_TEST(test_theme_parse_bad);

    printf(g_test_failures ? "TESTS FAILED: %d\n" : "ALL TESTS PASSED\n", g_test_failures);
    return g_test_failures ? 1 : 0;
}
