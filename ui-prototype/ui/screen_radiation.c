#include "screen_radiation.h"
#include "view_theme.h"
#include "fonts/fonts.h"
#include <stdio.h>

typedef struct {
    GaugeModel *model;
    lv_obj_t   *scale;
    lv_obj_t   *needle;
    lv_obj_t   *value_lbl;
    lv_obj_t   *timer_lbl;
    lv_obj_t   *max_lbl;
    lv_obj_t   *start_lbl;
} Rad;

static Rad r;

#define NEEDLE_LEN 52

static void start_evt(lv_event_t *e) {
    (void)e;
    if (r.model->running) gauge_reset(r.model);
    else                  gauge_start(r.model);
    lv_label_set_text(r.start_lbl, r.model->running ? "STOP" : "START");
    screen_radiation_update(NULL, r.model);
}

/* a bordered "button" the CLIP-BOY way */
static lv_obj_t *pip_button(lv_obj_t *parent, const char *txt, lv_event_cb_t cb) {
    const Palette *p = view_theme_palette();
    lv_obj_t *b = lv_button_create(parent);
    lv_obj_set_size(b, lv_pct(100), 26);
    lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(p->accent), 0);
    lv_obj_set_style_border_width(b, 2, 0);
    lv_obj_set_style_radius(b, 2, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(p->fg), 0);
    lv_obj_center(l);
    return l;
}

static lv_obj_t *info_line(lv_obj_t *parent, const char *txt) {
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(view_theme_palette()->fg), 0);
    return l;
}

lv_obj_t *screen_radiation_create(lv_obj_t *parent, GaugeModel *g) {
    const Palette *p = view_theme_palette();
    r.model = g;

    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* --- left: the dial --- */
    lv_obj_t *left = lv_obj_create(root);
    lv_obj_set_size(left, 150, lv_pct(100));
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_all(left, 0, 0);
    lv_obj_remove_flag(left, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *scale = lv_scale_create(left);
    r.scale = scale;
    lv_obj_set_size(scale, 140, 140);
    lv_obj_align(scale, LV_ALIGN_CENTER, 0, -6);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(scale, 0, 100);
    lv_scale_set_total_tick_count(scale, 21);
    lv_scale_set_major_tick_every(scale, 5);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_angle_range(scale, 270);
    lv_scale_set_rotation(scale, 135);
    lv_obj_set_style_line_color(scale, lv_color_hex(p->fg), LV_PART_MAIN);       /* arc */
    lv_obj_set_style_line_color(scale, lv_color_hex(p->fg), LV_PART_ITEMS);      /* minor ticks */
    lv_obj_set_style_line_color(scale, lv_color_hex(p->accent), LV_PART_INDICATOR); /* major ticks */
    lv_obj_set_style_text_color(scale, lv_color_hex(p->fg), LV_PART_INDICATOR);  /* labels */
    lv_obj_set_style_text_font(scale, &font_vt323_16, LV_PART_INDICATOR);

    r.needle = lv_line_create(scale);
    lv_obj_set_style_line_width(r.needle, 3, 0);
    lv_obj_set_style_line_color(r.needle, lv_color_hex(p->accent), 0);
    lv_obj_set_style_line_rounded(r.needle, true, 0);

    r.value_lbl = lv_label_create(scale);
    lv_obj_set_style_text_font(r.value_lbl, &font_vt323_24, 0);
    lv_obj_set_style_text_color(r.value_lbl, lv_color_hex(p->accent), 0);
    lv_label_set_text(r.value_lbl, "0");
    lv_obj_align(r.value_lbl, LV_ALIGN_CENTER, 0, 22);

    lv_obj_t *cap = lv_label_create(left);
    lv_label_set_text(cap, "Deauths/sec");
    lv_obj_set_style_text_color(cap, lv_color_hex(p->fg), 0);
    lv_obj_align(cap, LV_ALIGN_BOTTOM_MID, 0, 0);

    /* --- right: info + buttons --- */
    lv_obj_t *right = lv_obj_create(root);
    lv_obj_set_size(right, 128, lv_pct(100));
    lv_obj_set_style_bg_opa(right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_pad_all(right, 0, 0);
    lv_obj_set_style_pad_row(right, 3, 0);
    lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    r.start_lbl = pip_button(right, "START", start_evt);
    r.timer_lbl = info_line(right, "00:00:00");
    r.max_lbl   = info_line(right, "Max: 0/s");
    info_line(right, "Top ch: --");
    info_line(right, "Top BSSID:");
    info_line(right, "---");

    screen_radiation_update(root, g);
    return root;
}

void screen_radiation_update(lv_obj_t *screen, const GaugeModel *g) {
    (void)screen;
    if (!r.scale) return;
    lv_scale_set_line_needle_value(r.scale, r.needle, NEEDLE_LEN, g->value);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", g->value);
    lv_label_set_text(r.value_lbl, buf);
    lv_obj_align(r.value_lbl, LV_ALIGN_CENTER, 0, 22);
    uint32_t s = g->elapsed_ms / 1000;
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", s / 3600, (s / 60) % 60, s % 60);
    lv_label_set_text(r.timer_lbl, buf);
    snprintf(buf, sizeof(buf), "Max: %d/s", g->max);
    lv_label_set_text(r.max_lbl, buf);
}
