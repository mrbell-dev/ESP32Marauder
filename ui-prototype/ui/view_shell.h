#pragma once
#include "../lvgl/lvgl.h"
#include "nav.h"

/* Pip-Boy shell chrome: status bar + top sub-tab row + content area + bottom
 * page-tab row. Screens attach to view_shell_content(). Tab taps fire the
 * callbacks; the caller updates the NavState and calls view_shell_refresh(). */

typedef void (*shell_page_cb)(Page p);
typedef void (*shell_subtab_cb)(int idx);

lv_obj_t *view_shell_create(lv_obj_t *parent, const NavState *n,
                            shell_page_cb on_page, shell_subtab_cb on_subtab);
lv_obj_t *view_shell_content(lv_obj_t *shell);
void      view_shell_refresh(lv_obj_t *shell, const NavState *n);
