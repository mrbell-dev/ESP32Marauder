#pragma once
#include <stdio.h>

extern int g_test_failures;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s (%s:%d): %s\n", __func__, __FILE__, __LINE__, msg); \
        g_test_failures++; \
    } \
} while (0)

#define RUN_TEST(fn) do { printf("RUN  %s\n", #fn); fn(); } while (0)
