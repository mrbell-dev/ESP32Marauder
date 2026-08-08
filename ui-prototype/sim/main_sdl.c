/* CLIP-BOY 3000 — SDL entry point.
 * Interactive window, or headless --snapshot <name> [--page P --subtab S --theme F]. */
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/sdl/lv_sdl_window.h"
#include "lvgl/src/drivers/sdl/lv_sdl_mouse.h"
#include "sim/app.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "sim/third_party/stb_image_write.h"

#define BASE_W 320
#define BASE_H 240

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
        else if (!strcmp(argv[i], "--theme") && i + 1 < argc) app_set_theme_path(argv[++i]);
    }

    lv_init();

    if (snap) {
        make_headless();
        app_build_ui();
        if (page >= 0 || subtab >= 0) app_goto(page, subtab);
        if (app_current_is_radiation()) app_seed_gauge_demo();
        return write_snapshot(snap);
    }

    lv_tick_set_cb((lv_tick_get_cb_t)SDL_GetTicks);
    lv_display_t *disp = lv_sdl_window_create(BASE_W, BASE_H);
    lv_sdl_window_set_zoom(disp, 2.0f);
    lv_sdl_window_set_title(disp, "CLIP-BOY 3000 (sim)");
    lv_sdl_mouse_create();
    app_build_ui();
    app_start_feed_timer();
    app_show_boot();

    while (1) {
        uint32_t idle = lv_timer_handler();
        if (idle < 5) idle = 5;
        if (idle > 33) idle = 33;
        SDL_Delay(idle);
    }
    return 0;
}
