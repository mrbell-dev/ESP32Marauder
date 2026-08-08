# CLIP-BOY 3000 — Pip-Boy Marauder UI (POC)

Standalone LVGL prototype of a Fallout Pip-Boy skin for ESP32 Marauder. Presentation only —
no radio, no network. Renders at a true **320×240** (the CYD's screen), scaled up in the sim.

See the design + plan:
- `../docs/superpowers/specs/2026-08-08-pipboy-marauder-ui-design.md`
- `../docs/superpowers/plans/2026-08-08-pipboy-marauder-ui-poc.md`

## Layout
- `ui/` — pure-C UI model (nav, gauge, theme, tools). Zero LVGL includes; unit-tested.
- `sim/` — simulator entry points (`main_sdl.c`, later `main_web.c`).
- `tests/` — `test_runner` (pure-model unit tests) + `e2e/` (Playwright, Task 11).
- `themes/` — SD-style JSON color themes (Task 10).
- `lvgl/` — LVGL v9.5 submodule.

## Build & run (native SDL)
```bash
git submodule update --init --recursive
cmake -B build && cmake --build build
./build/test_runner    # unit tests
./build/sim            # the UI (640x480 window = 320x240 @2x)
```

Requires: cmake, a C11 compiler, SDL2 (`libsdl2-dev`).
