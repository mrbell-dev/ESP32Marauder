#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Pip-Boy color palette. Each field is 0xRRGGBB. */
typedef struct {
    uint32_t bg;      /* screen background          */
    uint32_t fg;      /* primary phosphor text      */
    uint32_t accent;  /* active-tab / highlight      */
    uint32_t dim;     /* inactive / separators       */
    uint32_t warn;    /* warnings / danger           */
} Palette;

/* The built-in default (Pip-Boy green). */
Palette palette_default_pipboy(void);

/* Parse a small JSON theme object like {"fg":"#33ff66","bg":"#0a1a0a"}.
 * Keys: bg, fg, accent, dim, warn. Unspecified keys keep *out's current value.
 * Returns false (leaving *out untouched) if the input is not a JSON object. */
bool theme_parse_json(const char *json, Palette *out);
