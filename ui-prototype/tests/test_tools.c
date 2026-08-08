#include "test_util.h"
#include "../ui/tools.h"
#include <string.h>

void test_tools_wifi_has_deauth(void) {
    ToolMenu m; toolmenu_wifi(&m);
    int found = 0;
    for (int c = 0; c < m.cat_count; c++)
        for (int t = 0; t < m.cats[c].tool_count; t++)
            if (strcmp(m.cats[c].tools[t].name, "Deauth") == 0) {
                toolmenu_select(&m, c, t);
                found = 1;
            }
    TEST_ASSERT(found, "Deauth present");
    const Tool *sel = toolmenu_selected(&m);
    TEST_ASSERT(sel && strcmp(sel->param_label, "Channel") == 0, "Deauth has Channel param");
    TEST_ASSERT(sel && sel->param_count == 4, "Channel has 4 opts");
}

void test_tools_toggle(void) {
    ToolMenu m; toolmenu_wifi(&m);
    bool before = m.cats[0].expanded;
    toolmenu_toggle(&m, 0);
    TEST_ASSERT(m.cats[0].expanded != before, "toggle flips expanded");
}

void test_tools_select_none_initially(void) {
    ToolMenu m; toolmenu_wifi(&m);
    TEST_ASSERT(toolmenu_selected(&m) == 0, "nothing selected at seed");
}
