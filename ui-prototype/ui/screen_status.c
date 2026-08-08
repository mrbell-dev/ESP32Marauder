#include "screen_status.h"
#include "view_theme.h"
#include "fonts/fonts.h"

static lv_obj_t *centered_col(lv_obj_t *parent) {
    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 4, 0);
    lv_obj_set_style_pad_row(root, 6, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return root;
}

lv_obj_t *screen_status_create(lv_obj_t *parent) {
    const Palette *p = view_theme_palette();
    lv_obj_t *root = centered_col(parent);

    lv_obj_t *title = lv_label_create(root);
    lv_label_set_text(title, "CLIP-BOY 3000");
    lv_obj_set_style_text_font(title, &font_vt323_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(p->accent), 0);

    lv_obj_t *coll = lv_label_create(root);
    lv_label_set_text(coll, "Collectibles: 0 / 95");
    lv_obj_set_style_text_color(coll, lv_color_hex(p->fg), 0);

    lv_obj_t *flavor = lv_label_create(root);
    lv_label_set_long_mode(flavor, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(flavor, lv_pct(90));
    lv_label_set_text(flavor,
        "It looks like you're surviving the digital apocalypse.\n\nWould you like help?");
    lv_obj_set_style_text_color(flavor, lv_color_hex(p->fg), 0);
    lv_obj_set_style_text_align(flavor, LV_TEXT_ALIGN_CENTER, 0);

    return root;
}

lv_obj_t *screen_stub_create(lv_obj_t *parent, const char *title) {
    const Palette *p = view_theme_palette();
    lv_obj_t *root = centered_col(parent);

    lv_obj_t *t = lv_label_create(root);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_font(t, &font_vt323_24, 0);
    lv_obj_set_style_text_color(t, lv_color_hex(p->accent), 0);

    lv_obj_t *s = lv_label_create(root);
    lv_label_set_text(s, "coming soon");
    lv_obj_set_style_text_color(s, lv_color_hex(p->dim), 0);

    return root;
}
