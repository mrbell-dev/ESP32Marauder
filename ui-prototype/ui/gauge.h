#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Deauths-per-second "radiation" gauge model (pure C, no LVGL). */
typedef struct {
    int      value;        /* current reading            */
    int      max;          /* peak seen since start      */
    bool     running;      /* Start/Reset state          */
    uint32_t elapsed_ms;   /* accrues while running      */
} GaugeModel;

void gauge_init(GaugeModel *g);                  /* zero all, not running */
void gauge_start(GaugeModel *g);                 /* running = true */
void gauge_reset(GaugeModel *g);                 /* zero all, not running */
void gauge_feed(GaugeModel *g, int v);           /* if running: value=v, max=max(max,v) */
void gauge_tick(GaugeModel *g, uint32_t dt_ms);  /* if running: elapsed_ms += dt_ms */
