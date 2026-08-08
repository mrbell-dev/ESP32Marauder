/* CLIP-BOY 3000 — LVGL Pip-Boy Marauder UI simulator.
 * Interactive SDL mode, or headless --snapshot <name> [--page P --subtab S]. */
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/sdl/lv_sdl_window.h"
#include "lvgl/src/drivers/sdl/lv_sdl_mouse.h"
#include "ui/theme.h"
#include "ui/view_theme.h"
#include "ui/view_shell.h"
#include "ui/nav.h"
#include "ui/gauge.h"
#include "ui/screen_radiation.h"
#include "ui/tools.h"
#include "ui/screen_tools.h"
#include "ui/screen_status.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "sim/third_party/stb_image_write.h"

#define BASE_W 320
#define BASE_H 240

static NavState   g_nav;
static lv_obj_t  *g_shell;
static GaugeModel g_gauge;
static ToolMenu   g_tools;

typedef enum { SCR_OTHER, SCR_RADIATION } CurScreen;
static CurScreen g_cur = SCR_OTHER;

/* Mount the screen for the current (page, subtab) into the shell content. */
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

static void build_ui(void) {
    Palette pal = palette_default_pipboy();
    view_theme_apply(&pal);
    nav_init(&g_nav);
    g_shell = view_shell_create(lv_screen_active(), &g_nav, on_page, on_subtab);
    seed_models();
    mount_current();                    /* STATS/Status by default */
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

static void show_boot(void) {
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

/* ---- headless snapshot ------------------------------------------------ */

static void noop_flush(lv_display_t *d, const lv_area_t *a, uint8_t *px) {
    (void)a; (void)px; lv_display_flush_ready(d);
}
static void make_headless(void) {
    static uint8_t buf[BASE_W * 48 * 2];
    lv_display_t *d = lv_display_create(BASE_W, BASE_H);
    lv_display_set_buffers(d, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d, noop_flush);
    lv_display_set_default(d);
}
static int write_snapshot(const char *name) {
    lv_obj_t *scr = lv_screen_active();
    lv_tick_inc(100);
    lv_refr_now(lv_display_get_default());
    lv_obj_update_layout(scr);
    lv_draw_buf_t *snap = lv_snapshot_take(scr, LV_COLOR_FORMAT_RGB888);
    if (!snap) { fprintf(stderr, "snapshot failed\n"); return 1; }
    for (uint32_t y = 0; y < snap->header.h; y++) {   /* BGR -> RGB */
        uint8_t *row = snap->data + (size_t)y * snap->header.stride;
        for (uint32_t x = 0; x < snap->header.w; x++) {
            uint8_t t = row[x * 3]; row[x * 3] = row[x * 3 + 2]; row[x * 3 + 2] = t;
        }
    }
    char path[256];
    snprintf(path, sizeof(path), "snapshots/%s.png", name);
    int ok = stbi_write_png(path, snap->header.w, snap->header.h, 3, snap->data, snap->header.stride);
    lv_draw_buf_destroy(snap);
    printf(ok ? "wrote %s\n" : "PNG write failed for %s\n", path);
    return ok ? 0 : 1;
}

/* ---- main ------------------------------------------------------------- */

int main(int argc, char **argv) {
    const char *snap = NULL;
    int page = -1, subtab = -1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--snapshot") && i + 1 < argc) snap = argv[++i];
        else if (!strcmp(argv[i], "--page") && i + 1 < argc) page = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--subtab") && i + 1 < argc) subtab = atoi(argv[++i]);
    }

    lv_init();

    if (snap) {
        make_headless();
        build_ui();
        if (page >= 0 && page < PAGE_COUNT) on_page((Page)page);
        if (subtab >= 0) on_subtab(subtab);
        if (g_cur == SCR_RADIATION) {           /* populate the dial */
            gauge_start(&g_gauge);
            gauge_feed(&g_gauge, 42);
            gauge_feed(&g_gauge, 71);
            gauge_feed(&g_gauge, 42);
            gauge_tick(&g_gauge, 125000);
            screen_radiation_update(NULL, &g_gauge);
        }
        return write_snapshot(snap);
    }

    lv_tick_set_cb((lv_tick_get_cb_t)SDL_GetTicks);
    lv_display_t *disp = lv_sdl_window_create(BASE_W, BASE_H);
    lv_sdl_window_set_zoom(disp, 2.0f);
    lv_sdl_window_set_title(disp, "CLIP-BOY 3000 (sim)");
    lv_sdl_mouse_create();
    build_ui();
    lv_timer_create(feed_timer, 500, NULL);
    show_boot();

    while (1) {
        uint32_t idle = lv_timer_handler();
        if (idle < 5) idle = 5;
        if (idle > 33) idle = 33;
        SDL_Delay(idle);
    }
    return 0;
}
