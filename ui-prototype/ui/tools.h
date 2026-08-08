#pragma once
#include <stdbool.h>

/* Tool list / detail model (pure C, no LVGL).
 * Mirrors Marauder's real WiFi menu grouped into Pip-Boy categories. */

typedef struct {
    const char *name;
    const char *desc;
    const char *param_label;    /* NULL if the tool has no parameter */
    const char *param_opts[4];  /* option strings; unused slots NULL  */
    int         param_count;
} Tool;

typedef struct {
    const char *name;
    const Tool *tools;
    int         tool_count;
    bool        expanded;
} ToolCategory;

typedef struct {
    ToolCategory *cats;
    int           cat_count;
    int           sel_cat;    /* -1 = none selected */
    int           sel_tool;   /* -1 = none selected */
} ToolMenu;

void        toolmenu_wifi(ToolMenu *m);                 /* seed WiFi categories */
void        toolmenu_toggle(ToolMenu *m, int cat);      /* expand/collapse a category */
void        toolmenu_select(ToolMenu *m, int cat, int tool);
const Tool *toolmenu_selected(const ToolMenu *m);       /* NULL if nothing selected */
