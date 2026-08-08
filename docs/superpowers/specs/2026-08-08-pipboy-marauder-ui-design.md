# Pip-Boy Marauder UI — Design

**Date:** 2026-08-08
**Status:** Approved (design), pending spec review
**Author:** Michael Bell (with Claude Code)

## One-line

Full reskin of ESP32 Marauder into a Fallout Pip-Boy ("CLIP-BOY") aesthetic — replace the
**presentation layer only**, leave the radio/scan/attack engine untouched. On-device, offline,
no server. Built with **LVGL**. Native C++ is the end goal; **POC first in the LVGL desktop
simulator**.

## Goals

- Recreate the CLIP-BOY Pip-Boy look: green-phosphor CRT, amber accents, 3-page tab nav,
  animated gauges, two-pane detail views, `> START <` buttons.
- Drive the UI from Marauder's **real menu structure** (WiFi / Bluetooth / Device functions),
  not CLIP-BOY's device-specific tabs.
- Build the UI **once** and run it in two places from the same code: a desktop simulator
  (fast iteration) and the CYD (real hardware).
- Ship an **SD-card theme** system so colors are swappable without reflashing.
- Keep the design portable and integratable back into the Marauder firmware.

## Non-goals

- **Do not touch the radio/scan/attack engine.** The valuable, hard-won part stays as-is; we
  only change how things are drawn and navigated.
- Not a web app; nothing served over HTTP. HTML cannot render on the ESP32 offline, and a
  web-server UI would tether the device to a phone — rejected.
- No **data-driven layout** (widget positions from a file). Layout lives in code. Themes cover
  colors (+ optional font selection), nothing more.
- POC is **standalone** — not yet wired to real radio functions (simulated data).

## Target hardware & constraints

- **CYD ESP32-2432S028** (the plain `cyd_2432S028` / internal `MARAUDER_CYD_MICRO` build) —
  ILI9341 **320×240**, XPT2046 resistive touch.
- Design at **true 320×240**, scaled up in the simulator but pixel-honest so it ports cleanly.
- **Footprint (flash/RAM) is an integration-phase risk to verify**, NOT a POC blocker. Marauder's
  app is ~1.5 MB of ~1.9 MB app space and uses significant RAM for the WiFi/BT stacks; LVGL adds
  flash (~100–300 KB) plus draw buffers. Verified before firmware merge, not during the POC.

## Why LVGL (decision record)

- Marauder's entire UI today is **TFT_eSPI direct-drawing** (no LVGL; `lv_arduino` is an unused
  dep). A full reskin **replaces** that UI wholesale, so "LVGL is foreign to the existing UI" stops
  mattering — we're not keeping the old UI.
- LVGL gives the gauge (`lv_arc` / `lv_meter`), sliders, tab bar, scrolling lists, styling and
  **animation** as built-in widgets, and has a **first-class SDL desktop simulator** (same code as
  the device). This is materially less effort per screen than hand-drawing against primitives.
- Rejected alternatives: **web POC** (can't run on-device, throwaway, violates standalone
  requirement); **thin TFT_eSPI draw-layer** (integrates natively but far more manual for the
  gauge/animation the mockups need).

## Architecture

- **LVGL** is the UI framework, replacing Marauder's menu/display layer.
- **Same UI code, two backends:**
  - **Simulator** — LVGL SDL driver on desktop. Fast iteration + screenshot testing.
  - **Device** — LVGL wired to TFT_eSPI (display flush callback) and XPT2046 (touch input
    callback). Well-trodden path for the CYD.
- **UI-diff isolation rule (critical for upstream sync):** all Pip-Boy UI lives in **new files**
  (a `ui/` module). Engine files are touched **minimally** — only to call existing functions from
  UI event handlers. This keeps `git merge upstream/master` clean: new upstream scan/attack
  *functions* merge for free; each new feature needs only a small UI entry (a list item + detail
  pane wired to it), not a rebuild.

## Navigation model (Pip-Boy 3-page, mapped to Marauder)

Bottom tabs = 3 pages (Pip-Boy STATS · ITEMS · DATA). Each page has top sub-tabs.

| Page | Sub-tabs | Content |
|------|----------|---------|
| **STATS** | Status · Radiation · GPS | Device status + mascot/flavor + collectible count; the Deauths/sec **radiation gauge** (animated needle, Start/Reset, timer, Max/s, Top ch, Top BSSID); GPS fix/sats/coords |
| **ITEMS** | WiFi · Bluetooth | Expandable category lists (`v Scanners`, `v Sniffers`, `v Attacks`; BT `v Sniffers`, `v Attacks`) + right-side **detail pane** (description, params e.g. channel dropdown, `> START <`). Populated with real Marauder functions (Ping/ARP/Port Scan, Deauth, Beacon Spam, Probe, Evil Portal, BLE scan, …) |
| **DATA** | Settings · Files | Device settings; Save/Load SSIDs/APs; Update Firmware |

Exact function→entry mapping is finalized in the implementation plan.

## Theme subsystem

- A JSON theme file on the SD card defines the palette (and optionally which built-in font):
  `{"bg":"#0a1a0a","fg":"#33ff66","accent":"#ffcc00","dim":"#1a331a","warn":"#ff5544"}`
- Read at boot (and re-applied from a Settings option); applied via **LVGL styles** at runtime —
  no reflash to change themes. LVGL's runtime styling makes this natural.
- **Pip-Boy green is the built-in default theme.** Amber (classic Fallout) and CLIP-BOY cyan are
  drop-in SD files.
- Scope: **colors first** (+ optional font *selection* from built-ins). No SD fonts at runtime in
  v1 (possible later via LVGL binary fonts, but heavier). No layout in the theme file.
- Fits Marauder's existing SD-config patterns (settings JSON, Evil Portal HTML) and the
  device-stands-alone / local-first ethos.

## Widget / component inventory (what the firmware port will need)

- Status bar (BAT %, indicator glyphs)
- Bottom page-tab bar (`●` active / `○` inactive)
- Top sub-tab bar (underline active)
- Expandable category list + two-pane detail view
- Gauge (`lv_arc` / `lv_meter`) — the radiation / deauths-per-sec meter, animated
- Sliders (brightness / R / G / B)
- Buttons (`> START <`, Reset)
- CRT overlay (scanlines + subtle glow) — background/overlay image, not a shader

## POC scope (Phase 1 — what we build now)

1. LVGL **desktop simulator** project that builds and runs.
2. **Full nav shell** — 3 pages, top sub-tabs, theme applied, brief boot flourish, status bar,
   page indicators.
3. **3 hero screens fully built:**
   - Radiation / Deauths-per-sec **gauge** (animated needle, Start/Reset, timer, Max/s).
   - One **ITEMS tool-detail flow** — expandable list + detail pane + params + `> START <`
     (candidate: **Deauth** or **Ping Scan**; pick in the plan).
   - **Status** screen (mascot + flavor + counts).
4. Remaining sub-tabs **navigable but stubbed** ("coming soon").
5. **Simulated data** — no radio; fake feeds drive the gauge and lists.
6. **Screenshot testing** — PNG snapshots per screen and/or Playwright-driven interaction on the
   sim window, checked against the CLIP-BOY reference stills.

## Integration path (Phase 2 — documented, not built now)

1. Bring LVGL into the Marauder fork; wire display-flush to TFT_eSPI and input to XPT2046.
2. Replace Marauder's `MenuFunctions` / `Display` UI entry points with the LVGL UI module.
3. Wire LVGL event handlers to the existing engine (`wifi_scan_obj` methods, etc.).
4. **Verify flash/RAM footprint** on the CYD; use `#ifdef` feature flags to trim what we don't
   ship and buy back space.
5. Keep the UI in new files, minimal engine edits → clean upstream merges going forward.

## Testing

- Simulator: LVGL SDL; per-screen PNG snapshots and/or Playwright interaction.
- Visual review against the CLIP-BOY reference stills (on Desktop).
- (Firmware phase) on-device smoke test + footprint check.

## Risks

- **Flash/RAM footprint** on the CYD alongside Marauder (integration phase — verify before merge).
- **CRT effect fidelity** in LVGL (overlay approach vs the easy CSS version).
- **LVGL + CYD driver setup** (well-documented, but first-time setup cost).

## Open questions (resolve in planning)

- Exact Marauder function → Pip-Boy entry mapping.
- Which tool-detail flow is the POC hero (Deauth vs Ping Scan).
- Simulator specifics: LVGL version, SDL vs PC-framebuffer, build system for the sim.
