import { test, expect } from '@playwright/test';

/* Screenshot tests for the CLIP-BOY WASM build. Each navigates via the
 * window.pipboyGoto(page, subtab) hook exposed by the app, then snapshots
 * the canvas. First run records baselines; later runs compare. */

const screens: Array<{ name: string; page: number; subtab: number }> = [
  { name: 'status',    page: 0, subtab: 0 },
  { name: 'radiation', page: 0, subtab: 1 },
  { name: 'deauth',    page: 1, subtab: 0 },
];

for (const s of screens) {
  test(`screen ${s.name}`, async ({ page }) => {
    await page.goto('/');
    // wait for the WASM runtime + JS hook to be ready
    await page.waitForFunction(() => (window as any).pipboyReady === true, null, {
      timeout: 30000,
    });
    await page.evaluate(
      ({ p, sub }) => (window as any).pipboyGoto(p, sub),
      { p: s.page, sub: s.subtab },
    );
    // let LVGL render a couple frames
    await page.waitForTimeout(700);
    await expect(page.locator('#canvas')).toHaveScreenshot(`${s.name}.png`, {
      maxDiffPixels: 300,
    });
  });
}
