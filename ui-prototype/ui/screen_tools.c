#include "screen_tools.h"
#include "view_theme.h"
#include "fonts/fonts.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    ToolMenu *model;
    lv_obj_t *list;      /* left column */
    lv_obj_t *detail;    /* right pane */
} Tools;

static Tools t;

static void rebuild(void);

/* ---- events ----------------------------------------------------------- */

static void cat_evt(lv_event_t *e) {
    int c = (int)(intptr_t)lv_event_get_user_data(e);
    toolmenu_toggle(t.model, c);
    rebuild();
}
static void tool_evt(lv_event_t *e) {
    int enc = (int)(intptr_t)lv_event_get_user_data(e);
    toolmenu_select(t.model, enc / 100, enc % 100);
    rebuild();
}
static void start_evt(lv_event_t *e) {
    (void)e;
    const Tool *s = toolmenu_selected(t.model);
    if (s) printf("START %s\n", s->name);   /* no radio; POC logs intent */
}

/* ---- row helper ------------------------------------------------------- */

static lv_obj_t *row(lv_obj_t *parent, const char *txt, lv_event_cb_t cb,
                     int idx, bool selected, int indent) {
    const Palette *p = view_theme_palette();
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_size(c, lv_pct(100), 22);
    lv_obj_set_style_bg_color(c, lv_color_hex(p->dim), 0);
    lv_obj_set_style_bg_opa(c, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(c, 0, 0);
    lv_obj_set_style_radius(c, 0, 0);
    lv_obj_set_style_pad_all(c, 0, 0);
    lv_obj_set_style_pad_left(c, 2 + indent, 0);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(c, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(c, cb, LV_EVENT_CLICKED, (void *)(intptr_t)idx);
    lv_obj_t *l = lv_label_create(c);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(selected ? p->accent : p->fg), 0);
    lv_obj_align(l, LV_ALIGN_LEFT_MID, 0, 0);
    return c;
}

/* ---- detail pane ------------------------------------------------------ */

static void build_detail(void) {
    const Palette *p = view_theme_palette();
    lv_obj_clean(t.detail);
    const Tool *sel = toolmenu_selected(t.model);
    if (!sel) {
        lv_obj_t *l = lv_label_create(t.detail);
        lv_label_set_text(l, "Select a tool");
        lv_obj_set_style_text_color(l, lv_color_hex(p->fg), 0);
        lv_obj_center(l);
        return;
    }
    lv_obj_t *title = lv_label_create(t.detail);
    lv_label_set_text(title, sel->name);
    lv_obj_set_style_text_font(title, &font_vt323_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(p->accent), 0);

    lv_obj_t *desc = lv_label_create(t.detail);
    lv_label_set_long_mode(desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc, lv_pct(100));
    lv_label_set_text(desc, sel->desc);
    lv_obj_set_style_text_color(desc, lv_color_hex(p->fg), 0);

    if (sel->param_label && sel->param_count > 0) {
        lv_obj_t *plabel = lv_label_create(t.detail);
        lv_label_set_text(plabel, sel->param_label);
        lv_obj_set_style_text_color(plabel, lv_color_hex(p->fg), 0);

        char opts[64] = {0};
        for (int i = 0; i < sel->param_count; i++) {
            strncat(opts, sel->param_opts[i], sizeof(opts) - strlen(opts) - 2);
            if (i + 1 < sel->param_count) strncat(opts, "\n", 2);
        }
        lv_obj_t *dd = lv_dropdown_create(t.detail);
        lv_dropdown_set_options(dd, opts);
        lv_obj_set_width(dd, lv_pct(100));
        lv_obj_set_style_bg_opa(dd, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(dd, lv_color_hex(p->accent), 0);
        lv_obj_set_style_border_width(dd, 2, 0);
        lv_obj_set_style_text_color(dd, lv_color_hex(p->fg), 0);
    }

    lv_obj_t *b = lv_button_create(t.detail);
    lv_obj_set_width(b, lv_pct(100));
    lv_obj_set_style_bg_color(b, lv_color_hex(p->accent), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, 2, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_add_event_cb(b, start_evt, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bl = lv_label_create(b);
    lv_label_set_text(bl, "> START <");
    lv_obj_set_style_text_color(bl, lv_color_hex(p->bg), 0);
    lv_obj_center(bl);
}

/* ---- list ------------------------------------------------------------- */

static void build_list(void) {
    lv_obj_clean(t.list);
    const ToolMenu *m = t.model;
    for (int c = 0; c < m->cat_count; c++) {
        char head[32];
        snprintf(head, sizeof(head), "%s %s", m->cats[c].expanded ? "v" : ">", m->cats[c].name);
        row(t.list, head, cat_evt, c, false, 0);
        if (m->cats[c].expanded) {
            for (int i = 0; i < m->cats[c].tool_count; i++) {
                bool sel = (m->sel_cat == c && m->sel_tool == i);
                row(t.list, m->cats[c].tools[i].name, tool_evt, c * 100 + i, sel, 12);
            }
        }
    }
}

static void rebuild(void) {
    build_list();
    build_detail();
}

/* ---- public ----------------------------------------------------------- */

lv_obj_t *screen_tools_create(lv_obj_t *parent, ToolMenu *m) {
    const Palette *p = view_theme_palette();
    t.model = m;

    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_pad_column(root, 6, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);

    t.list = lv_obj_create(root);
    lv_obj_set_size(t.list, 145, lv_pct(100));
    lv_obj_set_style_bg_opa(t.list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(t.list, lv_color_hex(p->accent), 0);
    lv_obj_set_style_border_width(t.list, 1, 0);
    lv_obj_set_style_border_side(t.list, LV_BORDER_SIDE_RIGHT, 0);
    lv_obj_set_style_radius(t.list, 0, 0);
    lv_obj_set_style_pad_all(t.list, 0, 0);
    lv_obj_set_style_pad_row(t.list, 1, 0);
    lv_obj_set_flex_flow(t.list, LV_FLEX_FLOW_COLUMN);

    t.detail = lv_obj_create(root);
    lv_obj_set_flex_grow(t.detail, 1);
    lv_obj_set_height(t.detail, lv_pct(100));
    lv_obj_set_style_bg_opa(t.detail, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(t.detail, 0, 0);
    lv_obj_set_style_radius(t.detail, 0, 0);
    lv_obj_set_style_pad_all(t.detail, 2, 0);
    lv_obj_set_style_pad_row(t.detail, 5, 0);
    lv_obj_set_flex_flow(t.detail, LV_FLEX_FLOW_COLUMN);

    rebuild();
    return root;
}

void screen_tools_refresh(lv_obj_t *screen, const ToolMenu *m) {
    (void)screen; (void)m;
    rebuild();
}
