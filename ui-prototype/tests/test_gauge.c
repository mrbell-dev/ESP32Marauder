#include "test_util.h"
#include "../ui/gauge.h"

void test_gauge_tracks_max_only_when_running(void) {
    GaugeModel g; gauge_init(&g);
    gauge_feed(&g, 40);
    TEST_ASSERT(g.value == 0 && g.max == 0, "ignored while stopped");
    gauge_start(&g);
    gauge_feed(&g, 40);
    gauge_feed(&g, 25);
    TEST_ASSERT(g.value == 25, "latest value");
    TEST_ASSERT(g.max == 40, "max held");
    gauge_tick(&g, 1000);
    TEST_ASSERT(g.elapsed_ms == 1000, "elapsed accrues");
    gauge_reset(&g);
    TEST_ASSERT(g.value == 0 && g.max == 0 && g.elapsed_ms == 0 && !g.running, "reset clears");
}
