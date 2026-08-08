#pragma once

/* Platform-independent CLIP-BOY app: builds the LVGL UI and drives navigation.
 * The SDL and web entry points both use this; only the main loop differs. */

void app_set_theme_path(const char *path);  /* NULL = built-in Pip-Boy green */
void app_build_ui(void);                    /* build shell + mount default screen */
void app_goto(int page, int subtab);        /* navigate; <0 keeps current */
int  app_current_is_radiation(void);        /* for snapshot seeding */
void app_seed_gauge_demo(void);             /* populate the dial for a snapshot */
void app_start_feed_timer(void);            /* animate the gauge (lv_timer) */
void app_show_boot(void);                   /* brief boot overlay */
