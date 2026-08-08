#pragma once
#include "../lvgl/lvgl.h"
#include "theme.h"

/* Apply a palette to the active screen (bg, default text color/font) and store
 * it so screens can query the current colors. */
void           view_theme_apply(const Palette *p);
const Palette *view_theme_palette(void);

/* Convenience: LVGL color from a palette field. */
static inline lv_color_t view_col(uint32_t rgb) { return lv_color_hex(rgb); }
