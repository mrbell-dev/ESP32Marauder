# Pip-Boy Marauder UI — Phase 2: Firmware Port (task list)

**Status:** not started. Phase 1 POC is complete on `feature/pipboy-ui`
(LVGL sim, SDL + WASM, all screens). This phase ports that UI onto the real CYD.

**Goal:** Replace Marauder's TFT_eSPI menu/display layer with the LVGL Pip-Boy UI on
the ESP32-2432S028 (CYD), wired to the real radio engine. Engine untouched; UI in new files
so upstream merges stay clean.

**Reference:** design doc `docs/superpowers/specs/2026-08-08-pipboy-marauder-ui-design.md`
(see "Integration path"); the working UI lives in `ui-prototype/ui/`.

---

## Task list

### A. De-risk footprint FIRST (do before anything else)
- [ ] Add LVGL v9.5 to the Marauder Arduino build for `MARAUDER_CYD_MICRO` (the working
      `cyd_2432S028` build); compile a hello-world LVGL screen alongside the full Marauder
      firmware and confirm it **fits flash and RAM** on the CYD. This is the make-or-break risk.
- [ ] If tight: use `#ifdef` to compile out unneeded Marauder features (badusb, extra scans)
      and/or LVGL widgets; re-measure. Decide go/no-go for LVGL vs. a lighter TFT_eSPI reskin.

### B. LVGL platform glue on the CYD
- [ ] Wire LVGL display flush callback to TFT_eSPI (ILI9341), 320x240, correct rotation.
- [ ] Wire LVGL input (XPT2046 touch) to an `lv_indev`; calibrate to the CYD panel.
- [ ] Provide `lv_tick` (millis) and an `lv_timer_handler` pump in the main loop without
      starving the radio tasks.

### C. Port the UI module (from `ui-prototype/ui/`)
- [ ] Copy the pure-model files as-is: `theme.c/.h`, `nav.c/.h`, `gauge.c/.h`, `tools.c/.h`.
- [ ] Copy the view files: `view_theme`, `view_shell`, `screen_radiation`, `screen_tools`,
      `screen_status`, plus the VT323 fonts. Keep them in a new `esp32_marauder/pipboy/` dir
      (new files → clean upstream merges).
- [ ] Add a build flag (e.g. `MARAUDER_PIPBOY_UI`) that swaps the LVGL UI in for the existing
      `MenuFunctions`/`Display` entry points.

### D. Wire the UI to the real engine
- [ ] Radiation screen: feed the gauge from the real deauth-sniff counter; Start/Reset drive
      the actual deauth-sniff scan; fill Top ch / Top BSSID from live data.
- [ ] Tools screen: map each list entry to a real `wifi_scan_obj` function; `> START <` launches
      the selected scan/attack with the chosen params (channel, etc.).
- [ ] Status screen: real device info (firmware version, free heap, AP/station counts).
- [ ] GPS sub-tab: real GPS fix/sats/coords (the GT-U7 already works on this board).
- [ ] Bluetooth / Settings / Files sub-tabs: wire to the corresponding Marauder menus.

### E. On-device theme + polish
- [ ] Load `/pipboy_theme.json` from the SD card at boot; apply via `view_theme`; fall back to
      built-in green (mirror the sim's `--theme`). Ship `amber.json` as an alt on the card.
- [ ] Optional: CRT scanline overlay (deferred in Phase 1) — tiled image, not a shader.
- [ ] Optional: proper filled/empty dot glyphs or keep the circle-shape markers.

### F. Verify + integrate
- [ ] On-device smoke test: every tab navigates, every wired function launches, no crash/reboot.
- [ ] Re-measure flash/RAM headroom; confirm stable under load (WiFi/BT active).
- [ ] Confirm `git merge upstream/master` stays clean (UI is all new files).
- [ ] Decide: keep as a personal build, or propose upstream as an optional `MARAUDER_PIPBOY_UI`.

## Open questions for Phase 2 kickoff
- Does LVGL + full Marauder actually fit on the CYD? (Task A answers this; everything depends on it.)
- Keep the whole Pip-Boy nav, or start by skinning just the scan screens?
- Personal fork build, or aim for upstream acceptance?
