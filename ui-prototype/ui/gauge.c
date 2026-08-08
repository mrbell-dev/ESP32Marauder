#include "gauge.h"

void gauge_init(GaugeModel *g) {
    g->value = 0;
    g->max = 0;
    g->running = false;
    g->elapsed_ms = 0;
}

void gauge_start(GaugeModel *g) {
    g->running = true;
}

void gauge_reset(GaugeModel *g) {
    gauge_init(g);
}

void gauge_feed(GaugeModel *g, int v) {
    if (!g->running) return;
    g->value = v;
    if (v > g->max) g->max = v;
}

void gauge_tick(GaugeModel *g, uint32_t dt_ms) {
    if (g->running) g->elapsed_ms += dt_ms;
}
