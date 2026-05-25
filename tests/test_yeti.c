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

int main(void) {
    test_smoke();
    fprintf(stderr, "\n%d/%d tests passed\n",
            test_count - fail_count, test_count);
    return fail_count ? 1 : 0;
}
