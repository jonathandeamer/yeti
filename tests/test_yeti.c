/* SPDX-License-Identifier: MIT OR CC0-1.0 */

#define GAME_TEST 1
#include "../src/yeti.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_count = 0;
static int fail_count = 0;

#define ASSERT(cond) do {                                                  \
    test_count++;                                                          \
    if (!(cond)) {                                                         \
        fprintf(stderr, "\nFAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
        fail_count++;                                                      \
    } else {                                                               \
        fputc('.', stdout);                                                \
        fflush(stdout);                                                    \
    }                                                                      \
} while (0)

static void test_smoke(void) {
    ASSERT(1 == 1);
}

static void test_xorshift32_deterministic(void) {
    unsigned int s1 = 1, s2 = 1;
    for (int i = 0; i < 100; i++) {
        ASSERT(xorshift32(&s1) == xorshift32(&s2));
    }
}

static void test_xorshift32_nontrivial(void) {
    unsigned int s = 1;
    unsigned int v1 = xorshift32(&s);
    unsigned int v2 = xorshift32(&s);
    unsigned int v3 = xorshift32(&s);
    ASSERT(v1 != 0 && v2 != 0 && v3 != 0);
    ASSERT(v1 != v2 && v2 != v3 && v1 != v3);
}

static void test_xorshift32_different_seeds(void) {
    unsigned int s1 = 1, s2 = 2;
    ASSERT(xorshift32(&s1) != xorshift32(&s2));
}

static void test_xorshift_uniform_in_range(void) {
    unsigned int s = 12345;
    for (int i = 0; i < 1000; i++) {
        unsigned int v = xorshift_uniform(&s, 100);
        ASSERT(v < 100);
    }
}

int main(void) {
    test_smoke();
    test_xorshift32_deterministic();
    test_xorshift32_nontrivial();
    test_xorshift32_different_seeds();
    test_xorshift_uniform_in_range();
    fprintf(stderr, "\n%d/%d tests passed\n",
            test_count - fail_count, test_count);
    return fail_count ? 1 : 0;
}
