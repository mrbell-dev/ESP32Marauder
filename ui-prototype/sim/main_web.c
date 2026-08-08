/* CLIP-BOY 3000 — Emscripten/WASM entry point (browser).
 * Same app + LVGL as the SDL build; browser main loop + a JS nav hook. */
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/sdl/lv_sdl_window.h"
#include "lvgl/src/drivers/sdl/lv_sdl_mouse.h"
#include "sim/app.h"
#include <emscripten.h>
#include <SDL2/SDL.h>

#define BASE_W 320
#define BASE_H 240

/* Playwright / JS hook: window.pipboyGoto(page, subtab). */
EMSCRIPTEN_KEEPALIVE void pipboy_goto(int page, int subtab) {
    app_goto(page, subtab);
}

static void main_loop(void) {
    lv_timer_handler();
}

int main(void) {
    lv_init();
    lv_tick_set_cb((lv_tick_get_cb_t)SDL_GetTicks);
    lv_display_t *disp = lv_sdl_window_create(BASE_W, BASE_H);
    lv_sdl_window_set_zoom(disp, 2.0f);
    lv_sdl_mouse_create();
    app_build_ui();
    app_start_feed_timer();
    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}
