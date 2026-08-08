#include "view_theme.h"
#include "fonts/fonts.h"

static Palette g_pal;

void view_theme_apply(const Palette *p) {
    g_pal = *p;
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(p->bg), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(scr, lv_color_hex(p->fg), 0);
    lv_obj_set_style_text_font(scr, &font_vt323_16, 0);
}

const Palette *view_theme_palette(void) {
    return &g_pal;
}
