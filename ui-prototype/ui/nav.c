#include "nav.h"

static const char *const PAGE_NAMES[PAGE_COUNT] = { "STATS", "ITEMS", "DATA" };

static const char *const SUBTABS[PAGE_COUNT][3] = {
    { "Status", "Radiation", "GPS" },   /* STATS: 3 */
    { "WiFi", "Bluetooth", 0 },          /* ITEMS: 2 */
    { "Settings", "Files", 0 },          /* DATA:  2 */
};

static const int SUBTAB_COUNTS[PAGE_COUNT] = { 3, 2, 2 };

void nav_init(NavState *n) {
    n->page = PAGE_STATS;
    for (int i = 0; i < PAGE_COUNT; i++) n->subtab[i] = 0;
}

void nav_set_page(NavState *n, Page p) {
    if (p >= 0 && p < PAGE_COUNT) n->page = p;
}

void nav_set_subtab(NavState *n, int idx) {
    int max = SUBTAB_COUNTS[n->page];
    if (idx < 0) idx = 0;
    if (idx >= max) idx = max - 1;
    n->subtab[n->page] = idx;
}

int nav_subtab_count(Page p) {
    return (p >= 0 && p < PAGE_COUNT) ? SUBTAB_COUNTS[p] : 0;
}

const char *nav_page_name(Page p) {
    return (p >= 0 && p < PAGE_COUNT) ? PAGE_NAMES[p] : "";
}

const char *nav_subtab_name(Page p, int idx) {
    if (p < 0 || p >= PAGE_COUNT) return "";
    if (idx < 0 || idx >= SUBTAB_COUNTS[p]) return "";
    return SUBTABS[p][idx];
}
