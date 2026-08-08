#include "test_util.h"
#include "../ui/theme.h"

void test_theme_default(void) {
    Palette p = palette_default_pipboy();
    TEST_ASSERT(p.fg == 0x33ff66, "default fg");
    TEST_ASSERT(p.bg == 0x0a1a0a, "default bg");
    TEST_ASSERT(p.accent == 0xffcc00, "default accent");
}

void test_theme_parse_ok(void) {
    Palette p = palette_default_pipboy();
    bool ok = theme_parse_json("{\"fg\":\"#112233\",\"accent\":\"#445566\"}", &p);
    TEST_ASSERT(ok, "parse ok");
    TEST_ASSERT(p.fg == 0x112233, "parsed fg");
    TEST_ASSERT(p.accent == 0x445566, "parsed accent");
    TEST_ASSERT(p.bg == 0x0a1a0a, "unspecified keeps default");
}

void test_theme_parse_bad(void) {
    Palette p = palette_default_pipboy();
    bool ok = theme_parse_json("not json", &p);
    TEST_ASSERT(!ok, "bad json rejected");
    TEST_ASSERT(p.fg == 0x33ff66, "palette untouched on failure");
}
