#include "test_util.h"

int g_test_failures = 0;

/* Test declarations */
void test_theme_default(void);
void test_theme_parse_ok(void);
void test_theme_parse_bad(void);
void test_nav_init(void);
void test_nav_switch(void);
void test_nav_subtab_clamp(void);
void test_nav_names(void);
void test_gauge_tracks_max_only_when_running(void);

int main(void) {
    RUN_TEST(test_theme_default);
    RUN_TEST(test_theme_parse_ok);
    RUN_TEST(test_theme_parse_bad);
    RUN_TEST(test_nav_init);
    RUN_TEST(test_nav_switch);
    RUN_TEST(test_nav_subtab_clamp);
    RUN_TEST(test_nav_names);
    RUN_TEST(test_gauge_tracks_max_only_when_running);

    printf(g_test_failures ? "TESTS FAILED: %d\n" : "ALL TESTS PASSED\n", g_test_failures);
    return g_test_failures ? 1 : 0;
}
