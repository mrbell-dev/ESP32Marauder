#include "theme.h"
#include <string.h>
#include <stdio.h>

Palette palette_default_pipboy(void) {
    Palette p;
    p.bg     = 0x0a1a0a;
    p.fg     = 0x33ff66;
    p.accent = 0xffcc00;
    p.dim    = 0x1a331a;
    p.warn   = 0xff5544;
    return p;
}

/* Parse one hex digit; returns -1 if not hex. */
static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Parse "#RRGGBB" starting at s; on success writes value and returns true. */
static bool parse_hex_color(const char *s, uint32_t *out) {
    if (*s != '#') return false;
    s++;
    uint32_t v = 0;
    for (int i = 0; i < 6; i++) {
        int d = hex_digit(s[i]);
        if (d < 0) return false;
        v = (v << 4) | (uint32_t)d;
    }
    *out = v;
    return true;
}

/* Find `"key"` then the next `#RRGGBB` string value and assign it. */
static void grab(const char *json, const char *key, uint32_t *field) {
    char needle[16];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *k = strstr(json, needle);
    if (!k) return;
    const char *q = strchr(k + strlen(needle), '#');
    if (!q) return;
    /* the '#' must be inside a nearby quoted value (within a few chars of the colon) */
    parse_hex_color(q, field);
}

bool theme_parse_json(const char *json, Palette *out) {
    if (!json || !strchr(json, '{')) return false;
    grab(json, "bg", &out->bg);
    grab(json, "fg", &out->fg);
    grab(json, "accent", &out->accent);
    grab(json, "dim", &out->dim);
    grab(json, "warn", &out->warn);
    return true;
}
