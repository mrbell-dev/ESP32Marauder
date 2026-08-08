#include "test_util.h"
#include "../ui/nav.h"
#include <string.h>

void test_nav_init(void) {
    NavState n; nav_init(&n);
    TEST_ASSERT(n.page == PAGE_STATS, "starts on STATS");
    TEST_ASSERT(n.subtab[PAGE_STATS] == 0, "subtab 0");
}

void test_nav_switch(void) {
    NavState n; nav_init(&n);
    nav_set_page(&n, PAGE_ITEMS); nav_set_subtab(&n, 1);
    TEST_ASSERT(n.page == PAGE_ITEMS, "on ITEMS");
    TEST_ASSERT(n.subtab[PAGE_ITEMS] == 1, "ITEMS subtab 1");
    nav_set_page(&n, PAGE_STATS);
    TEST_ASSERT(n.subtab[PAGE_STATS] == 0, "STATS remembers 0");
    nav_set_page(&n, PAGE_ITEMS);
    TEST_ASSERT(n.subtab[PAGE_ITEMS] == 1, "ITEMS remembers 1");
}

void test_nav_subtab_clamp(void) {
    NavState n; nav_init(&n);
    nav_set_page(&n, PAGE_ITEMS); nav_set_subtab(&n, 9);
    TEST_ASSERT(n.subtab[PAGE_ITEMS] == 1, "clamped to last (2 subtabs)");
}

void test_nav_names(void) {
    TEST_ASSERT(strcmp(nav_page_name(PAGE_DATA), "DATA") == 0, "page name");
    TEST_ASSERT(nav_subtab_count(PAGE_STATS) == 3, "STATS 3 subtabs");
    TEST_ASSERT(strcmp(nav_subtab_name(PAGE_STATS, 1), "Radiation") == 0, "STATS/1 Radiation");
}
