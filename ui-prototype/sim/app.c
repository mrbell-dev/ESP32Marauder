#include "app.h"
#include "../lvgl/lvgl.h"
#include "../ui/theme.h"
#include "../ui/view_theme.h"
#include "../ui/view_shell.h"
#include "../ui/nav.h"
#include "../ui/gauge.h"
#include "../ui/screen_radiation.h"
#include "../ui/tools.h"
#include "../ui/screen_tools.h"
#include "../ui/screen_status.h"
#include <stdio.h>
#include <stdlib.h>

static NavState   g_nav;
static lv_obj_t  *g_shell;
static GaugeModel g_gauge;
static ToolMenu   g_tools;

typedef enum { SCR_OTHER, SCR_RADIATION } CurScreen;
static CurScreen g_cur = SCR_OTHER;

static const char *g_theme_path = NULL;

static void load_theme(Palette *out) {
    *out = palette_default_pipboy();
    if (!g_theme_path) return;
    FILE *f = fopen(g_theme_path, "rb");
    if (!f) { fprintf(stderr, "theme: cannot open %s (using default)\n", g_theme_path); return; }
    char buf[512];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = 0;
    fclose(f);
    if (!theme_parse_json(buf, out))
        fprintf(stderr, "theme: parse failed for %s (using default)\n", g_theme_path);
}

static void mount_current(void) {
    lv_obj_t *c = view_shell_content(g_shell);
    lv_obj_clean(c);
    Page pg = g_nav.page;
    int  st = g_nav.subtab[pg];
    g_cur = SCR_OTHER;
    if (pg == PAGE_STATS && st == 0)       screen_status_create(c);
    else if (pg == PAGE_STATS && st == 1) { screen_radiation_create(c, &g_gauge); g_cur = SCR_RADIATION; }
    else if (pg == PAGE_STATS && st == 2)  screen_stub_create(c, "GPS");
    else if (pg == PAGE_ITEMS && st == 0)  screen_tools_create(c, &g_tools);
    else if (pg == PAGE_ITEMS && st == 1)  screen_stub_create(c, "Bluetooth");
    else if (pg == PAGE_DATA  && st == 0)  screen_stub_create(c, "Settings");
    else                                   screen_stub_create(c, "Files");
}

static void on_page(Page p)  { nav_set_page(&g_nav, p);   view_shell_refresh(g_shell, &g_nav); mount_current(); }
static void on_subtab(int i) { nav_set_subtab(&g_nav, i); view_shell_refresh(g_shell, &g_nav); mount_current(); }

static void seed_models(void) {
    gauge_init(&g_gauge);
    toolmenu_wifi(&g_tools);
    toolmenu_toggle(&g_tools, 2);       /* Attacks expanded */
    toolmenu_select(&g_tools, 2, 0);    /* Deauth selected  */
}

static void feed_timer(lv_timer_t *t) {
    (void)t;
    if (g_cur == SCR_RADIATION && g_gauge.running) {
        gauge_feed(&g_gauge, rand() % 100);
        gauge_tick(&g_gauge, 500);
        screen_radiation_update(NULL, &g_gauge);
    }
}

static void boot_done(lv_timer_t *t) {
    lv_obj_delete((lv_obj_t *)lv_timer_get_user_data(t));
    lv_timer_delete(t);
}

/* ---- public API ------------------------------------------------------- */

void app_set_theme_path(const char *path) { g_theme_path = path; }

void app_build_ui(void) {
    Palette pal;
    load_theme(&pal);
    view_theme_apply(&pal);
    nav_init(&g_nav);
    g_shell = view_shell_create(lv_screen_active(), &g_nav, on_page, on_subtab);
    seed_models();
    mount_current();
}

void app_goto(int page, int subtab) {
    if (page >= 0 && page < PAGE_COUNT) nav_set_page(&g_nav, (Page)page);
    if (subtab >= 0) nav_set_subtab(&g_nav, subtab);
    view_shell_refresh(g_shell, &g_nav);
    mount_current();
}

int app_current_is_radiation(void) { return g_cur == SCR_RADIATION; }

void app_seed_gauge_demo(void) {
    gauge_start(&g_gauge);
    gauge_feed(&g_gauge, 42);
    gauge_feed(&g_gauge, 71);
    gauge_feed(&g_gauge, 42);
    gauge_tick(&g_gauge, 125000);
    screen_radiation_update(NULL, &g_gauge);
}

void app_start_feed_timer(void) {
    lv_timer_create(feed_timer, 500, NULL);
}

void app_show_boot(void) {
    const Palette *p = view_theme_palette();
    lv_obj_t *o = lv_obj_create(lv_layer_top());
    lv_obj_set_size(o, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(o, lv_color_hex(p->bg), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_t *l = lv_label_create(o);
    lv_label_set_text(l, "CLIP-BOY 3000\n\nbooting...");
    lv_obj_set_style_text_color(l, lv_color_hex(p->accent), 0);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(l);
    lv_timer_t *bt = lv_timer_create(boot_done, 1200, o);
    lv_timer_set_repeat_count(bt, 1);
}
