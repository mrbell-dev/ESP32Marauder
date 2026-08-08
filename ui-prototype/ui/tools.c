#include "tools.h"

/* --- WiFi tool data (real Marauder functions, Pip-Boy grouping) --------- */

static const Tool SCANNERS[] = {
    { "Ping Scan", "Ping every host on the subnet.", 0, {0}, 0 },
    { "ARP Scan",  "Discover hosts via ARP on the subnet.", 0, {0}, 0 },
    { "Scan APs",  "List nearby access points.", 0, {0}, 0 },
};

static const Tool SNIFFERS[] = {
    { "Beacon",   "Sniff beacon frames.", 0, {0}, 0 },
    { "Probe",    "Sniff probe requests.", 0, {0}, 0 },
    { "Deauth Sniff", "Watch for deauth frames.", 0, {0}, 0 },
};

static const Tool ATTACKS[] = {
    { "Deauth",      "Sniff/flood deauth packets in the area.",
      "Channel", { "1", "6", "11", "All" }, 4 },
    { "Beacon Spam", "Flood fake beacon frames.", 0, {0}, 0 },
    { "Probe Flood", "Flood probe requests.", 0, {0}, 0 },
};

static ToolCategory WIFI_CATS[] = {
    { "Scanners", SCANNERS, 3, false },
    { "Sniffers", SNIFFERS, 3, false },
    { "Attacks",  ATTACKS,  3, false },
};

void toolmenu_wifi(ToolMenu *m) {
    m->cats = WIFI_CATS;
    m->cat_count = 3;
    m->sel_cat = -1;
    m->sel_tool = -1;
    for (int i = 0; i < m->cat_count; i++) m->cats[i].expanded = false;
}

void toolmenu_toggle(ToolMenu *m, int cat) {
    if (cat < 0 || cat >= m->cat_count) return;
    m->cats[cat].expanded = !m->cats[cat].expanded;
}

void toolmenu_select(ToolMenu *m, int cat, int tool) {
    if (cat < 0 || cat >= m->cat_count) return;
    if (tool < 0 || tool >= m->cats[cat].tool_count) return;
    m->sel_cat = cat;
    m->sel_tool = tool;
}

const Tool *toolmenu_selected(const ToolMenu *m) {
    if (m->sel_cat < 0 || m->sel_tool < 0) return 0;
    return &m->cats[m->sel_cat].tools[m->sel_tool];
}
