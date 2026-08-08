cmake_minimum_required(VERSION 3.16)
project(pipboy_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- LVGL ---------------------------------------------------------------
# Point LVGL at our lv_conf.h (kept next to this file).
set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE FILEPATH "" FORCE)
add_subdirectory(lvgl)

# --- SDL2 ---------------------------------------------------------------
find_package(SDL2 REQUIRED)
# LVGL's built-in SDL driver (LV_USE_SDL=1) needs the SDL2 headers while LVGL
# itself compiles, so expose them to the lvgl target.
if(TARGET SDL2::SDL2)
    target_link_libraries(lvgl PUBLIC SDL2::SDL2)
else()
    target_include_directories(lvgl PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(lvgl PUBLIC ${SDL2_LIBRARIES})
endif()

# --- Shared UI model sources (pure C, no LVGL) --------------------------
# Populated by later tasks; kept as a variable so both sim and tests link it.
set(UI_MODEL_SRCS
    ui/theme.c
    ui/nav.c
    ui/gauge.c
    ui/tools.c
)

# Test sources (unit tests for the pure model)
set(UI_TEST_SRCS
    tests/test_theme.c
    tests/test_nav.c
    tests/test_gauge.c
    tests/test_tools.c
)

# LVGL-dependent view sources (sim only, never in the pure-model tests)
set(UI_VIEW_SRCS
    ui/view_theme.c
    ui/view_shell.c
    ui/screen_radiation.c
    ui/screen_tools.c
    ui/screen_status.c
    ui/fonts/vt323_16.c
    ui/fonts/vt323_24.c
)

# --- Simulator (LVGL + SDL) ---------------------------------------------
add_executable(sim sim/main_sdl.c sim/app.c ${UI_MODEL_SRCS} ${UI_VIEW_SRCS})
target_include_directories(sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sim PRIVATE lvgl)
if(TARGET SDL2::SDL2)
    target_link_libraries(sim PRIVATE SDL2::SDL2)
else()
    target_include_directories(sim PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(sim PRIVATE ${SDL2_LIBRARIES})
endif()

# --- Test runner (pure model, no LVGL/SDL) ------------------------------
add_executable(test_runner tests/test_runner.c ${UI_TEST_SRCS} ${UI_MODEL_SRCS})
target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
