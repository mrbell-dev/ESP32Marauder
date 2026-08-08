#!/usr/bin/env bash
# Build the CLIP-BOY POC to WASM (browser). Requires Emscripten on PATH:
#   source ~/emsdk/emsdk_env.sh
set -e
cd "$(dirname "$0")"
mkdir -p web

emcc -I. -DLV_CONF_INCLUDE_SIMPLE \
  $(find lvgl/src -name '*.c') \
  ui/theme.c ui/nav.c ui/gauge.c ui/tools.c \
  ui/view_theme.c ui/view_shell.c \
  ui/screen_radiation.c ui/screen_tools.c ui/screen_status.c \
  ui/fonts/vt323_16.c ui/fonts/vt323_24.c \
  sim/app.c sim/main_web.c \
  -sUSE_SDL=2 \
  -sEXPORTED_FUNCTIONS=_main,_pipboy_goto \
  -sEXPORTED_RUNTIME_METHODS=ccall \
  -sALLOW_MEMORY_GROWTH=1 \
  -O2 \
  --shell-file sim/shell.html \
  -o web/index.html

echo "built web/index.html"
