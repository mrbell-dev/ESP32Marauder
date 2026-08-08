#pragma once

/* Pip-Boy 3-page navigation model (pure C, no LVGL). */

typedef enum { PAGE_STATS, PAGE_ITEMS, PAGE_DATA, PAGE_COUNT } Page;

typedef struct {
    Page page;                 /* current page                     */
    int  subtab[PAGE_COUNT];   /* remembered sub-tab per page      */
} NavState;

void        nav_init(NavState *n);                 /* page=STATS, subtabs=0 */
void        nav_set_page(NavState *n, Page p);
void        nav_set_subtab(NavState *n, int idx);  /* for current page; clamped */
int         nav_subtab_count(Page p);              /* STATS=3, ITEMS=2, DATA=2 */
const char *nav_page_name(Page p);                 /* "STATS"/"ITEMS"/"DATA" */
const char *nav_subtab_name(Page p, int idx);      /* e.g. STATS/0 -> "Status" */
