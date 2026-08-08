#include "view_shell.h"
#include "view_theme.h"
#include <stdio.h>

/* Single shell instance for the POC. */
typedef struct {
    lv_obj_t       *root;
    lv_obj_t       *subtab_row;
    lv_obj_t       *content;
    lv_obj_t       *pagetab_cell[PAGE_COUNT];
    lv_obj_t       *pagetab_lbl[PAGE_COUNT];
    shell_page_cb   on_page;
    shell_subtab_cb on_subtab;
} Shell;

static Shell g;

/* ---- helpers ---------------------------------------------------------- */

static lv_obj_t *bare_row(lv_obj_t *parent, int h) {
    lv_obj_t *r = lv_obj_create(parent);
    lv_obj_set_size(r, lv_pct(100), h);
    lv_obj_set_style_bg_opa(r, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(r, 0, 0);
    lv_obj_set_style_radius(r, 0, 0);
    lv_obj_set_style_pad_all(r, 0, 0);
    lv_obj_set_style_pad_column(r, 6, 0);
    lv_obj_remove_flag(r, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
    return r;
}

static void border_bottom(lv_obj_t *o, uint32_t rgb) {
    lv_obj_set_style_border_color(o, lv_color_hex(rgb), 0);
    lv_obj_set_style_border_width(o, 1, 0);
    lv_obj_set_style_border_side(o, LV_BORDER_SIDE_BOTTOM, 0);
}
static void border_top(lv_obj_t *o, uint32_t rgb) {
    lv_obj_set_style_border_color(o, lv_color_hex(rgb), 0);
    lv_obj_set_style_border_width(o, 1, 0);
    lv_obj_set_style_border_side(o, LV_BORDER_SIDE_TOP, 0);
}

static void page_evt(lv_event_t *e) {
    int p = (int)(intptr_t)lv_event_get_user_data(e);
    if (g.on_page) g.on_page((Page)p);
}
static void subtab_evt(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (g.on_subtab) g.on_subtab(i);
}

/* A clickable text cell. */
static lv_obj_t *tab_cell(lv_obj_t *parent, const char *txt,
                          lv_event_cb_t cb, int idx) {
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_size(c, LV_SIZE_CONTENT, lv_pct(100));
    lv_obj_set_style_bg_opa(c, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c, 0, 0);
    lv_obj_set_style_pad_hor(c, 4, 0);
    lv_obj_set_style_pad_ver(c, 0, 0);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(c, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(c, cb, LV_EVENT_CLICKED, (void *)(intptr_t)idx);
    lv_obj_t *l = lv_label_create(c);
    lv_label_set_text(l, txt);
    lv_obj_center(l);
    return c;
}

/* ---- public ----------------------------------------------------------- */

lv_obj_t *view_shell_create(lv_obj_t *parent, const NavState *n,
                            shell_page_cb on_page, shell_subtab_cb on_subtab) {
    const Palette *p = view_theme_palette();
    g.on_page = on_page;
    g.on_subtab = on_subtab;

    lv_obj_t *root = lv_obj_create(parent);
    g.root = root;
    lv_obj_set_size(root, 320, 240);
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);

    /* status bar */
    lv_obj_t *sb = bare_row(root, 18);
    lv_obj_set_flex_align(sb, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(sb, 4, 0);
    border_bottom(sb, p->dim);
    lv_obj_t *bat = lv_label_create(sb);
    lv_label_set_text(bat, "BAT 90%");
    lv_obj_t *fl = lv_label_create(sb);
    lv_label_set_text(fl, "? FL 146K");

    /* sub-tab row (filled by refresh) */
    g.subtab_row = bare_row(root, 22);
    lv_obj_set_flex_align(g.subtab_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(g.subtab_row, 4, 0);
    border_bottom(g.subtab_row, p->accent);

    /* content region */
    g.content = lv_obj_create(root);
    lv_obj_set_width(g.content, lv_pct(100));
    lv_obj_set_flex_grow(g.content, 1);
    lv_obj_set_style_bg_opa(g.content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g.content, 0, 0);
    lv_obj_set_style_radius(g.content, 0, 0);
    lv_obj_set_style_pad_all(g.content, 4, 0);
    lv_obj_remove_flag(g.content, LV_OBJ_FLAG_SCROLLABLE);

    /* bottom page-tab row */
    lv_obj_t *pt = bare_row(root, 20);
    lv_obj_set_flex_align(pt, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    border_top(pt, p->dim);
    for (int i = 0; i < PAGE_COUNT; i++) {
        g.pagetab_cell[i] = tab_cell(pt, "", page_evt, i);
        g.pagetab_lbl[i] = lv_obj_get_child(g.pagetab_cell[i], 0);
    }

    view_shell_refresh(root, n);
    return root;
}

lv_obj_t *view_shell_content(lv_obj_t *shell) {
    (void)shell;
    return g.content;
}

void view_shell_refresh(lv_obj_t *shell, const NavState *n) {
    (void)shell;
    const Palette *p = view_theme_palette();

    /* rebuild sub-tabs for the current page */
    lv_obj_clean(g.subtab_row);
    int count = nav_subtab_count(n->page);
    for (int i = 0; i < count; i++) {
        lv_obj_t *c = tab_cell(g.subtab_row, nav_subtab_name(n->page, i), subtab_evt, i);
        lv_obj_t *l = lv_obj_get_child(c, 0);
        bool active = (i == n->subtab[n->page]);
        lv_obj_set_style_text_color(l, active ? lv_color_hex(p->accent) : lv_color_hex(p->fg), 0);
        lv_obj_set_style_text_decor(l, active ? LV_TEXT_DECOR_UNDERLINE : LV_TEXT_DECOR_NONE, 0);
    }

    /* page-tab markers + highlight (ASCII so they render in Montserrat) */
    for (int i = 0; i < PAGE_COUNT; i++) {
        bool active = (i == n->page);
        char buf[24];
        snprintf(buf, sizeof(buf), "%s %s", active ? "(*)" : "( )", nav_page_name((Page)i));
        lv_label_set_text(g.pagetab_lbl[i], buf);
        lv_obj_set_style_text_color(g.pagetab_lbl[i], active ? lv_color_hex(p->accent) : lv_color_hex(p->fg), 0);
    }
}
