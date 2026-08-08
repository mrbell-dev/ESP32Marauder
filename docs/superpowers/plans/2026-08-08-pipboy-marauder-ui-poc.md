# Pip-Boy Marauder UI (POC) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a standalone LVGL Pip-Boy ("CLIP-BOY") UI POC for Marauder that runs in a desktop simulator (SDL) and a browser (WASM), with a full 3-page nav shell and three fully-built hero screens.

**Architecture:** A pure-C **UI model** (navigation, gauge, theme palette, tool list/detail) holds all state and logic with zero LVGL/rendering dependencies, so it is fully unit-testable. A thin **LVGL view** layer builds widgets that render the model and route events back into it. A **simulator main** feeds simulated data and drives the LVGL tick loop, built for both SDL (native, fast iteration) and Emscripten (WASM, browser — phone-viewable + Playwright-testable). No radio, no network — simulated data only.

**Tech Stack:** C11, LVGL v9, SDL2 (native driver), Emscripten (WASM driver), CMake, a tiny assert-based test runner (no framework).

## Global Constraints

- Base render resolution is **320×240** (the CYD's real screen); simulator scales up but pixel geometry stays honest.
- **UI model layer has zero LVGL includes** — pure C, so it compiles and tests without a display.
- All Pip-Boy UI lives under **`ui-prototype/`** (new files only); no Marauder engine files are touched in the POC.
- Default theme is **Pip-Boy green**: `bg #0a1a0a`, `fg #33ff66`, `accent #ffcc00`, `dim #1a331a`, `warn #ff5544`.
- Simulated data only — **no radio, no network, no web server**.
- Frequent commits: every task ends with a commit.

---

### Task 1: Project scaffold + test harness + blank SDL window

**Files:**
- Create: `ui-prototype/CMakeLists.txt`
- Create: `ui-prototype/sim/main_sdl.c`
- Create: `ui-prototype/lv_conf.h`
- Create: `ui-prototype/tests/test_runner.c`
- Create: `ui-prototype/tests/test_util.h`
- Create: `ui-prototype/.gitignore`
- Create: `ui-prototype/README.md`

**Interfaces:**
- Produces: a `test_runner` binary that runs registered tests and returns non-zero on failure; `TEST_ASSERT(cond, msg)` macro in `test_util.h`.

- [ ] **Step 1: Fetch LVGL v9 as a submodule**

Run:
```bash
cd ui-prototype && git submodule add -b release/v9.2 https://github.com/lvgl/lvgl.git lvgl
```

- [ ] **Step 2: Write `lv_conf.h`** (minimal: enable SDL/emscripten, 16-bit color, default 320x240, enable `lv_meter`, `lv_slider`, `lv_tabview`, `lv_list`, PNG/`lv_snapshot`).

Copy `lvgl/lv_conf_template.h` to `ui-prototype/lv_conf.h`, set `#if 1`, `LV_COLOR_DEPTH 16`, `LV_USE_SDL 1`, `LV_USE_MENU/METER/SLIDER/TABVIEW/LIST 1`, `LV_USE_SNAPSHOT 1`.

- [ ] **Step 3: Write the test harness** — `test_util.h`:

```c
#pragma once
#include <stdio.h>
extern int g_test_failures;
#define TEST_ASSERT(cond, msg) do { \
  if (!(cond)) { printf("FAIL: %s (%s:%d): %s\n", __func__, __FILE__, __LINE__, msg); g_test_failures++; } \
} while (0)
#define RUN_TEST(fn) do { printf("RUN  %s\n", #fn); fn(); } while (0)
```

`test_runner.c`:
```c
#include "test_util.h"
int g_test_failures = 0;
int main(void) {
  /* RUN_TEST calls added by later tasks */
  printf(g_test_failures ? "TESTS FAILED: %d\n" : "ALL TESTS PASSED\n", g_test_failures);
  return g_test_failures ? 1 : 0;
}
```

- [ ] **Step 4: Write `main_sdl.c`** — `lv_init()`, create SDL window driver at 320x240 (2x scale), fill screen with `bg`, run `lv_timer_handler()` loop. (Reference: `lvgl/examples/porting` + the SDL driver in `lvgl/src/drivers/sdl`.)

- [ ] **Step 5: Write `CMakeLists.txt`** with two targets: `sim` (LVGL + SDL2 + `main_sdl.c`) and `test_runner` (LVGL model sources + tests, no SDL). Find SDL2 via `find_package(SDL2)`.

- [ ] **Step 6: Build and run the test harness**

Run:
```bash
cd ui-prototype && cmake -B build && cmake --build build
./build/test_runner
```
Expected: prints `ALL TESTS PASSED`, exits 0.

- [ ] **Step 7: Build and run the sim**

Run: `./build/sim`
Expected: a 640×480 window (320×240 @2x) filled dark green appears.

- [ ] **Step 8: Commit**

```bash
git add ui-prototype
git commit -m "poc: LVGL+SDL scaffold, blank window, test harness"
```

---

### Task 2: Theme model (pure C — parse + defaults)

**Files:**
- Create: `ui-prototype/ui/theme.h`
- Create: `ui-prototype/ui/theme.c`
- Create: `ui-prototype/tests/test_theme.c`

**Interfaces:**
- Produces:
  ```c
  typedef struct { uint32_t bg, fg, accent, dim, warn; } Palette; // 0xRRGGBB
  Palette palette_default_pipboy(void);
  bool theme_parse_json(const char *json, Palette *out); // false on parse error; *out untouched on false
  ```

- [ ] **Step 1: Write the failing test** — `test_theme.c`:

```c
#include "test_util.h"
#include "../ui/theme.h"
void test_theme_default(void){
  Palette p = palette_default_pipboy();
  TEST_ASSERT(p.fg == 0x33ff66, "default fg");
  TEST_ASSERT(p.bg == 0x0a1a0a, "default bg");
}
void test_theme_parse_ok(void){
  Palette p = palette_default_pipboy();
  bool ok = theme_parse_json("{\"fg\":\"#112233\",\"accent\":\"#445566\"}", &p);
  TEST_ASSERT(ok, "parse ok");
  TEST_ASSERT(p.fg == 0x112233, "parsed fg");
  TEST_ASSERT(p.accent == 0x445566, "parsed accent");
  TEST_ASSERT(p.bg == 0x0a1a0a, "unspecified keeps default");
}
void test_theme_parse_bad(void){
  Palette p = palette_default_pipboy();
  bool ok = theme_parse_json("not json", &p);
  TEST_ASSERT(!ok, "bad json rejected");
  TEST_ASSERT(p.fg == 0x33ff66, "palette untouched on failure");
}
```
Register these with `RUN_TEST` in `test_runner.c`.

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build && ./build/test_runner`
Expected: FAIL (theme.h not found / functions undefined).

- [ ] **Step 3: Implement `theme.h` / `theme.c`** — the struct + `palette_default_pipboy()` returning the Global Constraints colors; `theme_parse_json` a tiny hand-rolled scanner that finds `"key":"#hex"` for keys bg/fg/accent/dim/warn (no JSON lib — YAGNI), parses `#RRGGBB` to `uint32_t`, returns false if the string has no `{`.

- [ ] **Step 4: Run to verify it passes**

Run: `cmake --build build && ./build/test_runner`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/theme.* ui-prototype/tests/test_theme.c ui-prototype/tests/test_runner.c
git commit -m "poc: theme model (palette + json parse) with tests"
```

---

### Task 3: Navigation model (pure C)

**Files:**
- Create: `ui-prototype/ui/nav.h`
- Create: `ui-prototype/ui/nav.c`
- Create: `ui-prototype/tests/test_nav.c`

**Interfaces:**
- Produces:
  ```c
  typedef enum { PAGE_STATS, PAGE_ITEMS, PAGE_DATA, PAGE_COUNT } Page;
  typedef struct { Page page; int subtab[PAGE_COUNT]; } NavState;
  void nav_init(NavState *n);           // page=STATS, all subtabs=0
  void nav_set_page(NavState *n, Page p);
  void nav_set_subtab(NavState *n, int idx);      // sets subtab for current page
  int  nav_subtab_count(Page p);        // STATS=3, ITEMS=2, DATA=2
  const char *nav_page_name(Page p);    // "STATS"/"ITEMS"/"DATA"
  const char *nav_subtab_name(Page p, int idx);   // e.g. STATS/0 -> "Status"
  ```

- [ ] **Step 1: Write the failing test** — `test_nav.c`:

```c
#include "test_util.h"
#include "../ui/nav.h"
#include <string.h>
void test_nav_init(void){
  NavState n; nav_init(&n);
  TEST_ASSERT(n.page == PAGE_STATS, "starts on STATS");
  TEST_ASSERT(n.subtab[PAGE_STATS] == 0, "subtab 0");
}
void test_nav_switch(void){
  NavState n; nav_init(&n);
  nav_set_page(&n, PAGE_ITEMS); nav_set_subtab(&n, 1);
  TEST_ASSERT(n.page == PAGE_ITEMS, "on ITEMS");
  TEST_ASSERT(n.subtab[PAGE_ITEMS] == 1, "ITEMS subtab 1");
  nav_set_page(&n, PAGE_STATS);
  TEST_ASSERT(n.subtab[PAGE_STATS] == 0, "STATS remembers 0");
  nav_set_page(&n, PAGE_ITEMS);
  TEST_ASSERT(n.subtab[PAGE_ITEMS] == 1, "ITEMS remembers 1");
}
void test_nav_names(void){
  TEST_ASSERT(strcmp(nav_page_name(PAGE_DATA), "DATA")==0, "page name");
  TEST_ASSERT(nav_subtab_count(PAGE_STATS)==3, "STATS 3 subtabs");
  TEST_ASSERT(strcmp(nav_subtab_name(PAGE_STATS,1), "Radiation")==0, "STATS/1 Radiation");
}
```
Register with `RUN_TEST`.

- [ ] **Step 2: Run to verify it fails.** Run: `cmake --build build && ./build/test_runner` → FAIL.

- [ ] **Step 3: Implement `nav.c`** — subtab names: STATS={"Status","Radiation","GPS"}, ITEMS={"WiFi","Bluetooth"}, DATA={"Settings","Files"}. `nav_set_subtab` clamps to `nav_subtab_count`.

- [ ] **Step 4: Run to verify it passes.** → PASS.

- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/nav.* ui-prototype/tests/test_nav.c ui-prototype/tests/test_runner.c
git commit -m "poc: navigation model with tests"
```

---

### Task 4: Gauge model (pure C)

**Files:**
- Create: `ui-prototype/ui/gauge.h`
- Create: `ui-prototype/ui/gauge.c`
- Create: `ui-prototype/tests/test_gauge.c`

**Interfaces:**
- Produces:
  ```c
  typedef struct { int value, max; bool running; uint32_t elapsed_ms; } GaugeModel;
  void gauge_init(GaugeModel *g);            // zeros
  void gauge_start(GaugeModel *g);           // running=true
  void gauge_reset(GaugeModel *g);           // zeros all
  void gauge_feed(GaugeModel *g, int v);     // if running: value=v, max=MAX(max,v)
  void gauge_tick(GaugeModel *g, uint32_t dt_ms); // if running: elapsed_ms += dt_ms
  ```

- [ ] **Step 1: Write the failing test** — `test_gauge.c`:

```c
#include "test_util.h"
#include "../ui/gauge.h"
void test_gauge_tracks_max_only_when_running(void){
  GaugeModel g; gauge_init(&g);
  gauge_feed(&g, 40);
  TEST_ASSERT(g.value==0 && g.max==0, "ignored while stopped");
  gauge_start(&g);
  gauge_feed(&g, 40); gauge_feed(&g, 25);
  TEST_ASSERT(g.value==25, "latest value");
  TEST_ASSERT(g.max==40, "max held");
  gauge_tick(&g, 1000);
  TEST_ASSERT(g.elapsed_ms==1000, "elapsed accrues");
  gauge_reset(&g);
  TEST_ASSERT(g.value==0 && g.max==0 && g.elapsed_ms==0 && !g.running, "reset clears");
}
```
Register with `RUN_TEST`.

- [ ] **Step 2: Run to verify it fails.** → FAIL.
- [ ] **Step 3: Implement `gauge.c`.**
- [ ] **Step 4: Run to verify it passes.** → PASS.
- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/gauge.* ui-prototype/tests/test_gauge.c ui-prototype/tests/test_runner.c
git commit -m "poc: gauge model with tests"
```

---

### Task 5: Tool list/detail model (pure C)

**Files:**
- Create: `ui-prototype/ui/tools.h`
- Create: `ui-prototype/ui/tools.c`
- Create: `ui-prototype/tests/test_tools.c`

**Interfaces:**
- Produces:
  ```c
  typedef struct { const char *name, *desc; const char *param_label; const char *param_opts[4]; int param_count; } Tool;
  typedef struct { const char *name; const Tool *tools; int tool_count; bool expanded; } ToolCategory;
  typedef struct { ToolCategory *cats; int cat_count; int sel_cat, sel_tool; } ToolMenu;
  void toolmenu_wifi(ToolMenu *m);          // seeds WiFi cats incl. Attacks->Deauth (param "Channel": 1/6/11, All)
  void toolmenu_toggle(ToolMenu *m, int cat);
  void toolmenu_select(ToolMenu *m, int cat, int tool);
  const Tool *toolmenu_selected(const ToolMenu *m); // NULL if none
  ```

- [ ] **Step 1: Write the failing test** — `test_tools.c`:

```c
#include "test_util.h"
#include "../ui/tools.h"
#include <string.h>
void test_tools_wifi_has_deauth(void){
  ToolMenu m; toolmenu_wifi(&m);
  int found=0;
  for(int c=0;c<m.cat_count;c++) for(int t=0;t<m.cats[c].tool_count;t++)
    if(strcmp(m.cats[c].tools[t].name,"Deauth")==0){ toolmenu_select(&m,c,t); found=1; }
  TEST_ASSERT(found, "Deauth present");
  const Tool *sel = toolmenu_selected(&m);
  TEST_ASSERT(sel && strcmp(sel->param_label,"Channel")==0, "Deauth has Channel param");
}
void test_tools_toggle(void){
  ToolMenu m; toolmenu_wifi(&m);
  bool before = m.cats[0].expanded;
  toolmenu_toggle(&m, 0);
  TEST_ASSERT(m.cats[0].expanded != before, "toggle flips expanded");
}
```
Register with `RUN_TEST`.

- [ ] **Step 2: Run to verify it fails.** → FAIL.
- [ ] **Step 3: Implement `tools.c`** — static WiFi categories: Scanners{Ping Scan, ARP Scan, Scan APs}, Sniffers{Beacon, Probe, Deauth-sniff}, Attacks{Deauth, Beacon Spam, Probe Flood}. Deauth tool: desc "Sniff/flood deauth packets in the area.", param_label "Channel", opts {"1","6","11","All"}.
- [ ] **Step 4: Run to verify it passes.** → PASS.
- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/tools.* ui-prototype/tests/test_tools.c ui-prototype/tests/test_runner.c
git commit -m "poc: tool list/detail model with tests"
```

---

### Task 6: Theme → LVGL styles + shell chrome (view)

**Files:**
- Create: `ui-prototype/ui/view_theme.h` / `.c`
- Create: `ui-prototype/ui/view_shell.h` / `.c`
- Modify: `ui-prototype/sim/main_sdl.c`

**Interfaces:**
- Consumes: `Palette`, `NavState`, `nav_*`.
- Produces:
  ```c
  void view_theme_apply(const Palette *p);     // builds/updates shared lv_style_t objects
  lv_obj_t *view_shell_create(lv_obj_t *parent, NavState *n,
      void (*on_page)(Page), void (*on_subtab)(int)); // status bar + top sub-tabs + bottom page tabs + content area
  lv_obj_t *view_shell_content(lv_obj_t *shell); // the empty content region screens attach to
  void view_shell_refresh(lv_obj_t *shell, const NavState *n); // updates active tab highlight + sub-tab labels
  ```

- [ ] **Step 1: Build the shell** — status bar row (`BAT 90%` left placeholder, `FL` right), top sub-tab row (labels from `nav_subtab_name`, active one underlined in `accent`), a content container, bottom page-tab row (`STATS ITEMS DATA` with `●/○` markers). Wire tab clicks to the `on_page`/`on_subtab` callbacks. Add a full-screen scanline overlay (semi-transparent horizontal lines) on top.

- [ ] **Step 2: Wire into `main_sdl.c`** — load default palette, `view_theme_apply`, create shell with callbacks that update a `NavState` and call `view_shell_refresh`.

- [ ] **Step 3: Build + run**

Run: `cmake --build build && ./build/sim`
Expected: Pip-Boy frame — dark-green bg, green text, amber active tab underline, three bottom tabs with one filled dot, faint scanlines. Clicking tabs moves the highlight.

- [ ] **Step 4: Snapshot check** — add `--snapshot stats` arg path to `main_sdl.c` that renders one frame to `snapshots/shell_stats.png` via `lv_snapshot_take` and exits.

Run: `./build/sim --snapshot stats` → writes `ui-prototype/snapshots/shell_stats.png`. Open it; confirm the frame matches the reference chrome.

- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/view_theme.* ui-prototype/ui/view_shell.* ui-prototype/sim/main_sdl.c
git commit -m "poc: theme styles + Pip-Boy shell chrome (tabs, status bar, scanlines)"
```

---

### Task 7: Radiation gauge screen (view)

**Files:**
- Create: `ui-prototype/ui/screen_radiation.h` / `.c`
- Modify: `ui-prototype/sim/main_sdl.c`

**Interfaces:**
- Consumes: `GaugeModel`, `gauge_*`, shell content region.
- Produces:
  ```c
  lv_obj_t *screen_radiation_create(lv_obj_t *parent, GaugeModel *g); // meter + Start/Reset + timer/Max labels
  void screen_radiation_update(lv_obj_t *screen, const GaugeModel *g); // needle + labels follow model
  ```

- [ ] **Step 1: Build the screen** — `lv_meter` arc 0–100 labelled "Deauths/sec" with a radiation-symbol image/emoji in the center, needle bound to `g->value`; right column: `Reset`/`Start` buttons (call `gauge_reset`/`gauge_start`), `00:00:00` timer from `elapsed_ms`, `Max: N/s`, `Top ch: --`, `Top BSSID: ---`.

- [ ] **Step 2: Simulated feed in `main_sdl.c`** — a timer that, while `g.running`, calls `gauge_feed(&g, rand()%100)` every 500ms and `gauge_tick`, then `screen_radiation_update`.

- [ ] **Step 3: Build + run**

Run: `./build/sim` → navigate STATS▸Radiation. Press Start; needle sweeps, timer counts, Max tracks the peak. Press Reset; all zero.

- [ ] **Step 4: Snapshot check**

Run: `./build/sim --snapshot radiation` → `snapshots/radiation.png`; compare to `Screenshot_20260808_133922.png` layout.

- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/screen_radiation.* ui-prototype/sim/main_sdl.c
git commit -m "poc: radiation/deauths-per-sec gauge screen"
```

---

### Task 8: Deauth tool-detail screen (view)

**Files:**
- Create: `ui-prototype/ui/screen_tools.h` / `.c`
- Modify: `ui-prototype/sim/main_sdl.c`

**Interfaces:**
- Consumes: `ToolMenu`, `toolmenu_*`, shell content region.
- Produces:
  ```c
  lv_obj_t *screen_tools_create(lv_obj_t *parent, ToolMenu *m);   // left expandable list + right detail pane
  void screen_tools_refresh(lv_obj_t *screen, const ToolMenu *m); // rebuild list + detail from model
  ```

- [ ] **Step 1: Build the screen** — left `lv_list`: categories with `v`/`>` prefix; expanding shows tools indented (`toolmenu_toggle`/`toolmenu_select` on click). Right detail pane: selected tool name (title), `desc`, a `param_label` + `lv_dropdown` of `param_opts`, a `> START <` button (on click: log "START <tool> ch <opt>" to stdout — no radio).

- [ ] **Step 2: Wire in `main_sdl.c`** — ITEMS▸WiFi renders this screen from a `ToolMenu` seeded by `toolmenu_wifi`.

- [ ] **Step 3: Build + run**

Run: `./build/sim` → ITEMS▸WiFi. Expand Attacks, select Deauth; detail shows the description, a Channel dropdown (1/6/11/All), and `> START <`. Clicking START prints the line.

- [ ] **Step 4: Snapshot check**

Run: `./build/sim --snapshot deauth` → `snapshots/deauth.png`; compare to `Screenshot_20260808_134741.png`.

- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/screen_tools.* ui-prototype/sim/main_sdl.c
git commit -m "poc: WiFi tools list + Deauth detail pane"
```

---

### Task 9: Status screen + stubs + full wiring

**Files:**
- Create: `ui-prototype/ui/screen_status.h` / `.c`
- Create: `ui-prototype/ui/screen_stub.h` / `.c`
- Modify: `ui-prototype/sim/main_sdl.c`

**Interfaces:**
- Produces:
  ```c
  lv_obj_t *screen_status_create(lv_obj_t *parent); // "CLIP-BOY 3000", Collectibles n/95, flavor text, mascot glyph
  lv_obj_t *screen_stub_create(lv_obj_t *parent, const char *title); // "<title>\ncoming soon"
  ```

- [ ] **Step 1: Build `screen_status`** (STATS▸Status) and `screen_stub` (generic).

- [ ] **Step 2: Central router in `main_sdl.c`** — on any nav change, clear the shell content and create the screen for `(page,subtab)`: STATS/0→status, STATS/1→radiation, STATS/2→stub "GPS", ITEMS/0→tools, ITEMS/1→stub "Bluetooth", DATA/*→stub. Add a 1-second boot flourish (green "CLIP-BOY 3000 booting…" then fade to STATS/Status).

- [ ] **Step 3: Build + run** — every bottom tab and sub-tab is reachable; unbuilt ones say "coming soon"; boot flourish shows on start.

- [ ] **Step 4: Snapshot check** — capture `status`, `stub` screens; verify all nav paths render without crash.

- [ ] **Step 5: Commit**

```bash
git add ui-prototype/ui/screen_status.* ui-prototype/ui/screen_stub.* ui-prototype/sim/main_sdl.c
git commit -m "poc: status screen, stubs, boot flourish, full nav routing"
```

---

### Task 10: SD-theme demo (load palette from file)

**Files:**
- Create: `ui-prototype/themes/pipboy.json`
- Create: `ui-prototype/themes/amber.json`
- Modify: `ui-prototype/sim/main_sdl.c`

**Interfaces:**
- Consumes: `theme_parse_json`, `view_theme_apply`.

- [ ] **Step 1: Write theme files** — `pipboy.json` (default colors), `amber.json` (`bg #1a1200`, `fg #ffb642`, `accent #ffe08a`, `dim #332600`, `warn #ff5544`).

- [ ] **Step 2: Load in `main_sdl.c`** — accept `--theme <path>`; read file, `theme_parse_json`, `view_theme_apply`; on parse failure fall back to `palette_default_pipboy()` and print a warning (proves the device's SD-load path in the sim).

- [ ] **Step 3: Build + run**

Run: `./build/sim --theme themes/amber.json` → whole UI re-skins amber, no code change. `./build/sim --theme bogus` → warns, stays green.

- [ ] **Step 4: Commit**

```bash
git add ui-prototype/themes ui-prototype/sim/main_sdl.c
git commit -m "poc: file-loaded color theme (green default + amber), proves SD theme path"
```

---

### Task 11: Browser (WASM) build + Playwright screenshot test

**Files:**
- Create: `ui-prototype/sim/main_web.c`
- Create: `ui-prototype/web/index.html`
- Create: `ui-prototype/web/CMakeLists-emscripten.md` (build notes)
- Create: `ui-prototype/tests/e2e/screens.spec.ts`
- Create: `ui-prototype/tests/e2e/playwright.config.ts`

**Interfaces:**
- Consumes: the same `ui/` model + view code as the SDL build.

- [ ] **Step 1: Emscripten driver** — `main_web.c` = the same UI setup as `main_sdl.c` but using LVGL's emscripten/`lv_sdl`-on-web canvas driver (reference `lvgl/examples` emscripten port). Document the `emcmake cmake … && emmake make` build in the `.md`.

- [ ] **Step 2: Build to WASM**

Run: `emcmake cmake -B build-web -DLV_BUILD_TARGET=web && cmake --build build-web`
Expected: produces `web/index.html` + `.wasm` + `.js`; opening it renders the Pip-Boy UI on a canvas.

- [ ] **Step 3: Write the Playwright test** — `screens.spec.ts`: serve `web/`, for each screen navigate (click the tab canvas coordinates or expose a `window.pipboyGoto(page,subtab)` hook from `main_web.c`), `await expect(page).toHaveScreenshot()`.

```ts
import { test, expect } from '@playwright/test';
const screens = ['status','radiation','deauth'];
for (const s of screens) {
  test(`screen ${s}`, async ({ page }) => {
    await page.goto('/');
    await page.evaluate((name) => (window as any).pipboyGoto(name), s);
    await expect(page.locator('canvas')).toHaveScreenshot(`${s}.png`);
  });
}
```

- [ ] **Step 4: Run Playwright**

Run: `cd ui-prototype/tests/e2e && npx playwright test`
Expected: first run writes baseline screenshots; second run passes against them.

- [ ] **Step 5: Publish preview** — note in README: the built `web/index.html` is self-contained and can be opened on a phone / published as an artifact for review.

- [ ] **Step 6: Commit**

```bash
git add ui-prototype/sim/main_web.c ui-prototype/web ui-prototype/tests/e2e
git commit -m "poc: WASM browser build + Playwright screenshot tests"
```

---

## Self-Review

**Spec coverage:**
- Pip-Boy aesthetic / CRT / tabs → Tasks 6–9. ✓
- Marauder menu structure in nav → Tasks 3, 5, 8, 9. ✓
- Build-once, sim + device same code → pure model (2–5) + view (6–9) + SDL (1) + WASM (11); device driver is the documented Phase-2 port. ✓
- SD theme system → Tasks 2, 10. ✓
- 320×240 base → Task 1 constraint, carried throughout. ✓
- POC scope: shell + 3 hero screens + stubs → Tasks 6–9. ✓
- Simulated data only → Tasks 7 (feed), 8 (START logs, no radio). ✓
- Testing (Playwright + snapshots) → Tasks 6–9 PNG snapshots, Task 11 Playwright. ✓ (Corrected: Playwright drives the WASM build, not SDL.)
- UI-diff isolation (all under `ui-prototype/`, no engine edits) → Global Constraints + every task. ✓

**Placeholder scan:** model tasks (2–5) have full test + impl guidance; view tasks (6–11) are inherently visual and use build-run + PNG/Playwright snapshot verification rather than unit asserts — deliberate, not placeholder. No "TBD"/"handle edge cases"/"similar to Task N".

**Type consistency:** `Palette`, `NavState`/`Page`, `GaugeModel`, `ToolMenu`/`Tool`/`ToolCategory` names and signatures are consistent across their defining task and consumers (6–9). `theme_parse_json`, `view_theme_apply`, `nav_*`, `gauge_*`, `toolmenu_*` used with matching signatures throughout.

**Note:** Emscripten (Task 11) has the only external-toolchain risk; if `emsdk` proves painful, the SDL build + PNG snapshots (Tasks 1–10) fully cover the POC, and browser preview can be deferred — Task 11 is isolated so it can slip without blocking.
