/* CLIP-BOY 3000 — LVGL Pip-Boy Marauder UI simulator.
 * Interactive SDL mode, or headless --snapshot <name> -> snapshots/<name>.png. */
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/sdl/lv_sdl_window.h"
#include "lvgl/src/drivers/sdl/lv_sdl_mouse.h"
#include "ui/theme.h"
#include "ui/view_theme.h"
#include "ui/view_shell.h"
#include "ui/nav.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "sim/third_party/stb_image_write.h"

#define BASE_W 320
#define BASE_H 240

static NavState g_nav;
static lv_obj_t *g_shell;

static void on_page(Page p)   { nav_set_page(&g_nav, p);   view_shell_refresh(g_shell, &g_nav); }
static void on_subtab(int i)  { nav_set_subtab(&g_nav, i); view_shell_refresh(g_shell, &g_nav); }

static void build_ui(void) {
    Palette pal = palette_default_pipboy();
    view_theme_apply(&pal);
    nav_init(&g_nav);
    g_shell = view_shell_create(lv_screen_active(), &g_nav, on_page, on_subtab);
}

/* ---- headless snapshot ------------------------------------------------ */

static void noop_flush(lv_display_t *d, const lv_area_t *a, uint8_t *px) {
    (void)a; (void)px; lv_display_flush_ready(d);
}

static void make_headless(void) {
    static uint8_t buf[BASE_W * 48 * 2]; /* partial band buffer, 16bpp */
    lv_display_t *d = lv_display_create(BASE_W, BASE_H);
    lv_display_set_buffers(d, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d, noop_flush);
    lv_display_set_default(d);
}

static int write_snapshot(const char *name) {
    lv_obj_t *scr = lv_screen_active();
    lv_tick_inc(100);
    lv_refr_now(lv_display_get_default());   /* compute coords via a real refresh */
    lv_obj_update_layout(scr);
    printf("screen %dx%d\n", (int)lv_obj_get_width(scr), (int)lv_obj_get_height(scr));
    lv_draw_buf_t *snap = lv_snapshot_take(scr, LV_COLOR_FORMAT_RGB888);
    if (!snap) { fprintf(stderr, "snapshot failed\n"); return 1; }
    /* LVGL stores RGB888 as B,G,R; swap to R,G,B for the PNG writer. */
    for (uint32_t y = 0; y < snap->header.h; y++) {
        uint8_t *row = snap->data + (size_t)y * snap->header.stride;
        for (uint32_t x = 0; x < snap->header.w; x++) {
            uint8_t t = row[x * 3]; row[x * 3] = row[x * 3 + 2]; row[x * 3 + 2] = t;
        }
    }
    char path[256];
    snprintf(path, sizeof(path), "snapshots/%s.png", name);
    int ok = stbi_write_png(path, snap->header.w, snap->header.h, 3,
                            snap->data, snap->header.stride);
    lv_draw_buf_destroy(snap);
    printf(ok ? "wrote %s\n" : "PNG write failed for %s\n", path);
    return ok ? 0 : 1;
}

/* ---- main ------------------------------------------------------------- */

int main(int argc, char **argv) {
    const char *snap = NULL;
    int page = -1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--snapshot") && i + 1 < argc) snap = argv[++i];
        else if (!strcmp(argv[i], "--page") && i + 1 < argc) page = atoi(argv[++i]);
    }

    lv_init();

    if (snap) {
        make_headless();          /* no SDL tick cb; driven by lv_tick_inc */
        build_ui();
        if (page >= 0 && page < PAGE_COUNT) on_page((Page)page);
        return write_snapshot(snap);
    }

    lv_tick_set_cb((lv_tick_get_cb_t)SDL_GetTicks);
    lv_display_t *disp = lv_sdl_window_create(BASE_W, BASE_H);
    lv_sdl_window_set_zoom(disp, 2.0f);
    lv_sdl_window_set_title(disp, "CLIP-BOY 3000 (sim)");
    lv_sdl_mouse_create();
    build_ui();

    while (1) {
        uint32_t idle = lv_timer_handler();
        if (idle < 5) idle = 5;
        if (idle > 33) idle = 33;
        SDL_Delay(idle);
    }
    return 0;
}
