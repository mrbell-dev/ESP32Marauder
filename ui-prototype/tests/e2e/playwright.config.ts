import { defineConfig } from '@playwright/test';

/* Serves the built WASM bundle (../../web) and runs screenshot tests against it.
 * Prereq: build the web bundle first (`source ~/emsdk/emsdk_env.sh && ./build-web.sh`)
 * and install browsers once (`npx playwright install chromium`). */
export default defineConfig({
  testDir: '.',
  snapshotDir: './__screenshots__',
  use: {
    baseURL: 'http://localhost:8099',
    viewport: { width: 700, height: 560 },
  },
  webServer: {
    command: 'python3 -m http.server 8099 --directory ../../web',
    port: 8099,
    reuseExistingServer: true,
  },
});
