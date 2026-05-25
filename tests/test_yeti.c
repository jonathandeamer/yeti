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

static void test_add_ms_basic(void) {
    struct timespec t = { .tv_sec = 5, .tv_nsec = 0 };
    add_ms(&t, 1500);
    ASSERT(t.tv_sec == 6);
    ASSERT(t.tv_nsec == 500000000);
}

static void test_add_ms_wrap(void) {
    struct timespec t = { .tv_sec = 5, .tv_nsec = 900000000 };
    add_ms(&t, 200);
    ASSERT(t.tv_sec == 6);
    ASSERT(t.tv_nsec == 100000000);
}

static void test_ms_until_positive(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    struct timespec future = now;
    add_ms(&future, 50);
    int ms = ms_until(future);
    ASSERT(ms > 0 && ms <= 50);
}

static void test_ms_until_past_returns_nonpositive(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    struct timespec past = now;
    past.tv_sec -= 1;
    int ms = ms_until(past);
    ASSERT(ms <= 0);
}

static void test_world_clear_zeros_buffer(void) {
    struct game g;
    g.world_top = 0;
    memset(g.world, 'X', sizeof(g.world));
    world_clear(&g);
    for (int r = 0; r < BUF_H; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            ASSERT(g.world[r][c] == 0);
        }
    }
}

static void test_world_get_set_roundtrip(void) {
    struct game g;
    g.world_top = 0;
    world_clear(&g);
    world_set(&g, 3, 17, 'T');
    ASSERT(world_get(&g, 3, 17) == 'T');
    ASSERT(world_get(&g, 3, 18) == 0);
    ASSERT(world_get(&g, 4, 17) == 0);
}

static void test_world_get_set_honors_ring_top(void) {
    struct game g;
    g.world_top = 5;
    world_clear(&g);
    world_set(&g, 0, 10, 'T');
    /* row 0 from caller's view is ring index (5 + 0) % BUF_H = 5 */
    ASSERT(g.world[5][10] == 'T');
    ASSERT(world_get(&g, 0, 10) == 'T');
}

static void test_world_get_set_wraps_ring(void) {
    struct game g;
    g.world_top = BUF_H - 2;
    world_clear(&g);
    world_set(&g, 5, 20, 'T');
    /* (BUF_H - 2 + 5) mod BUF_H = 3 */
    ASSERT(g.world[3][20] == 'T');
    ASSERT(world_get(&g, 5, 20) == 'T');
}

static int chunk_bit(int chunk, int row, int col) {
    int byte = col / 8;
    int bit  = 7 - (col % 8);
    return (chunk_pool[chunk][row][byte] >> bit) & 1;
}

static void test_chunk_count_matches_tiers(void) {
    int tiers[3] = { 0, 0, 0 };
    for (int i = 0; i < CHUNK_COUNT; i++) {
        ASSERT(chunk_tier[i] <= 2);
        tiers[chunk_tier[i]]++;
    }
    ASSERT(tiers[0] == 3);
    ASSERT(tiers[1] == 3);
    ASSERT(tiers[2] == 2);
}

static void test_chunk_last_row_always_empty(void) {
    for (int i = 0; i < CHUNK_COUNT; i++) {
        for (int b = 0; b < CHUNK_BYTES; b++) {
            ASSERT(chunk_pool[i][CHUNK_ROWS - 1][b] == 0);
        }
    }
}

static void test_chunk_no_three_consecutive_trees(void) {
    for (int chunk = 0; chunk < CHUNK_COUNT; chunk++) {
        for (int row = 0; row < CHUNK_ROWS; row++) {
            int run = 0;
            for (int col = 0; col < PLAYFIELD_W; col++) {
                if (chunk_bit(chunk, row, col)) {
                    run++;
                    ASSERT(run < 3);
                } else {
                    run = 0;
                }
            }
        }
    }
}

static void test_chunk_unused_bits_are_zero(void) {
    /* cols 60..63 must be unset (we only use 60 cols) */
    for (int chunk = 0; chunk < CHUNK_COUNT; chunk++) {
        for (int row = 0; row < CHUNK_ROWS; row++) {
            ASSERT((chunk_pool[chunk][row][CHUNK_BYTES - 1] & 0x0F) == 0);
        }
    }
}

static void test_chunk_pick_returns_tier0_when_distance_zero(void) {
    unsigned int s = 1;
    for (int i = 0; i < 50; i++) {
        int pick = chunk_pick(&s, 0);
        ASSERT(chunk_tier[pick] == 0);
    }
}

static void test_chunk_pick_returns_tier_1_or_2_when_distance_high(void) {
    unsigned int s = 1;
    int seen0 = 0, seen1 = 0, seen2 = 0;
    for (int i = 0; i < 200; i++) {
        int pick = chunk_pick(&s, 300);
        if (chunk_tier[pick] == 0) seen0 = 1;
        if (chunk_tier[pick] == 1) seen1 = 1;
        if (chunk_tier[pick] == 2) seen2 = 1;
    }
    ASSERT(seen0 == 0);
    ASSERT(seen1 == 1);
    ASSERT(seen2 == 1);
}

static void test_chunk_unpack_to_world(void) {
    struct game g;
    g.world_top = 0;
    world_clear(&g);
    chunk_unpack(&g, /*dest_row=*/0, /*chunk_idx=*/0);
    /* chunk 0: trees at (0,27), (1,12), (1,45), (2,40), row 3 empty */
    ASSERT(world_get(&g, 0, 27) == 'T');
    ASSERT(world_get(&g, 0, 26) == 0);
    ASSERT(world_get(&g, 0, 28) == 0);
    ASSERT(world_get(&g, 1, 12) == 'T');
    ASSERT(world_get(&g, 1, 45) == 'T');
    ASSERT(world_get(&g, 2, 40) == 'T');
    for (int c = 0; c < PLAYFIELD_W; c++) {
        ASSERT(world_get(&g, 3, c) == 0);
    }
}

static void test_world_scroll_advances_top(void) {
    struct game g;
    memset(&g, 0, sizeof g);
    int before = g.world_top;
    world_scroll(&g);
    ASSERT(g.world_top == (before + 1) % BUF_H);
}

static void test_world_gen_writes_chunk_with_empty_last_row(void) {
    struct game g;
    memset(&g, 0, sizeof g);
    g.rng = 1;
    world_clear(&g);
    int written = world_gen_chunk(&g);
    ASSERT(written >= 0);
    /* The chunk's last row (caller-relative written+CHUNK_ROWS-1) should be empty. */
    for (int c = 0; c < PLAYFIELD_W; c++) {
        ASSERT(world_get(&g, written + CHUNK_ROWS - 1, c) == 0);
    }
}

int main(void) {
    test_smoke();
    test_xorshift32_deterministic();
    test_xorshift32_nontrivial();
    test_xorshift32_different_seeds();
    test_xorshift_uniform_in_range();
    test_add_ms_basic();
    test_add_ms_wrap();
    test_ms_until_positive();
    test_ms_until_past_returns_nonpositive();
    test_world_clear_zeros_buffer();
    test_world_get_set_roundtrip();
    test_world_get_set_honors_ring_top();
    test_world_get_set_wraps_ring();
    test_chunk_count_matches_tiers();
    test_chunk_last_row_always_empty();
    test_chunk_no_three_consecutive_trees();
    test_chunk_unused_bits_are_zero();
    test_chunk_pick_returns_tier0_when_distance_zero();
    test_chunk_pick_returns_tier_1_or_2_when_distance_high();
    test_chunk_unpack_to_world();
    test_world_scroll_advances_top();
    test_world_gen_writes_chunk_with_empty_last_row();
    fprintf(stderr, "\n%d/%d tests passed\n",
            test_count - fail_count, test_count);
    return fail_count ? 1 : 0;
}
