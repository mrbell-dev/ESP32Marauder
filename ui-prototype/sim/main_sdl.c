/* CLIP-BOY 3000 — LVGL Pip-Boy Marauder UI, SDL simulator entry point.
 * Task 1: blank themed window + label, proving the LVGL+SDL toolchain. */
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/sdl/lv_sdl_window.h"
#include "lvgl/src/drivers/sdl/lv_sdl_mouse.h"
#include <SDL2/SDL.h>
#include <stdint.h>

#define BASE_W 320
#define BASE_H 240
#define BG   0x0a1a0a
#define FG   0x33ff66

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    lv_init();
    lv_tick_set_cb((lv_tick_get_cb_t)SDL_GetTicks);

    lv_display_t *disp = lv_sdl_window_create(BASE_W, BASE_H);
    lv_sdl_window_set_zoom(disp, 2.0f);
    lv_sdl_window_set_title(disp, "CLIP-BOY 3000 (sim)");
    lv_sdl_mouse_create();

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *lbl = lv_label_create(scr);
    lv_label_set_text(lbl, "CLIP-BOY 3000");
    lv_obj_set_style_text_color(lbl, lv_color_hex(FG), 0);
    lv_obj_center(lbl);

    while (1) {
        uint32_t idle = lv_timer_handler();
        if (idle < 5) idle = 5;
        if (idle > 33) idle = 33;
        SDL_Delay(idle);
    }
    return 0;
}
