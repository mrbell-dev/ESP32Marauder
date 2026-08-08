#pragma once
#include "../lvgl/lvgl.h"
#include "tools.h"

/* WiFi tools screen: left expandable category/tool list + right detail pane
 * (description, parameter dropdown, START). Attaches into the shell content. */
lv_obj_t *screen_tools_create(lv_obj_t *parent, ToolMenu *m);
void      screen_tools_refresh(lv_obj_t *screen, const ToolMenu *m);
