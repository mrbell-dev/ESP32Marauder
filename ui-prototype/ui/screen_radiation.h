#pragma once
#include "../lvgl/lvgl.h"
#include "gauge.h"

/* Radiation / Deauths-per-sec gauge screen. Attaches into the shell content
 * area. `g` is the model driving the needle + labels (owned by the caller). */
lv_obj_t *screen_radiation_create(lv_obj_t *parent, GaugeModel *g);
void      screen_radiation_update(lv_obj_t *screen, const GaugeModel *g);
