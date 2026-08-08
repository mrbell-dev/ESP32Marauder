#pragma once
#include "../lvgl/lvgl.h"

/* STATS/Status: device identity, collectible count, flavor text. */
lv_obj_t *screen_status_create(lv_obj_t *parent);

/* Generic "coming soon" placeholder for not-yet-built sub-tabs. */
lv_obj_t *screen_stub_create(lv_obj_t *parent, const char *title);
