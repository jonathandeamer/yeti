/* SPDX-License-Identifier: MIT OR CC0-1.0 */
#define _POSIX_C_SOURCE 200809L

/* --- includes --- */
#include <ncurses.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* --- test visibility --- */
#ifdef GAME_TEST
#define LOCAL
#else
#define LOCAL static
#endif

/* --- constants --- */
#define FRAME_MS         33
#define PLAYFIELD_W      60
#define PLAYFIELD_X_OFF  10
#define PLAYER_ROW       7
#define BUF_H            32
#define FP_SCALE         256
#define CHUNK_COUNT      8
#define CHUNK_ROWS       4
#define CHUNK_BYTES      8
#define REVEAL_MIN_S     30
#define REVEAL_MAX_S     60
#define BASE_CLOSING_FP  5
#define PRESS_COST_FP    30
#define SCROLL_PERIOD_START  6
#define SCROLL_PERIOD_END    3

#define PAIR_TREE    1
#define PAIR_PLAYER  2
#define PAIR_YETI    3
#define PAIR_HUD     4

/* --- state --- */
struct game {
    unsigned int rng;
    int tick;
    int distance;
    int player_col;
    int alive;
    int yeti_armed;
    int yeti_reveal_tick;
    int yeti_row_fp;
    int yeti_col;
    int scroll_period;
    int ticks_since_scroll;
    char world[BUF_H][PLAYFIELD_W];
    int world_top;
};

/* --- time --- */
LOCAL void add_ms(struct timespec *t, long ms) {
    t->tv_sec += ms / 1000;
    t->tv_nsec += (ms % 1000) * 1000000L;
    if (t->tv_nsec >= 1000000000L) {
        t->tv_sec += 1;
        t->tv_nsec -= 1000000000L;
    }
}

LOCAL int ms_until(struct timespec t) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long sec = t.tv_sec - now.tv_sec;
    long nsec = t.tv_nsec - now.tv_nsec;
    return (int)(sec * 1000 + nsec / 1000000);
}

/* --- prng --- */
LOCAL unsigned int xorshift32(unsigned int *s) {
    unsigned int x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

LOCAL unsigned int xorshift_uniform(unsigned int *s, unsigned int n) {
    /* Slight modulo bias acceptable for game variance use. */
    return xorshift32(s) % n;
}

/* --- world --- */
/* Bit layout: byte B covers cols B*8..B*8+7; bit 7 = leftmost (col B*8). */
LOCAL const uint8_t chunk_pool[CHUNK_COUNT][CHUNK_ROWS][CHUNK_BYTES] = {
    /* chunk 0 (tier 0): trees at (0,27), (1,12), (1,45), (2,40) */
    { { 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00 },
      { 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 1 (tier 0): (0,7), (0,45), (1,25), (2,18), (2,49) */
    { { 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x40, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 2 (tier 0): (0,18), (1,6), (1,37), (2,26) */
    { { 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 },
      { 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 3 (tier 1): (0,5),(0,19),(0,38) | (1,12),(1,29),(1,46) | (2,8),(2,21),(2,45) */
    { { 0x04, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00 },
      { 0x00, 0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00 },
      { 0x00, 0x80, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 4 (tier 1): (0,2),(0,15),(0,36) | (1,8),(1,27),(1,47) | (2,4),(2,18),(2,36) */
    { { 0x20, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 },
      { 0x00, 0x80, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00 },
      { 0x08, 0x00, 0x20, 0x00, 0x08, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 5 (tier 1): (0,7),(0,22),(0,41) | (1,2),(1,20),(1,45) | (2,11),(2,31),(2,48) */
    { { 0x01, 0x00, 0x02, 0x00, 0x00, 0x40, 0x00, 0x00 },
      { 0x20, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x10, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 6 (tier 2): (0,3),(0,10),(0,22),(0,38),(0,50) | (1,6),(1,16),(1,31),(1,46) | (2,1),(2,13),(2,23),(2,41),(2,52) */
    { { 0x10, 0x20, 0x02, 0x00, 0x02, 0x00, 0x20, 0x00 },
      { 0x02, 0x00, 0x80, 0x01, 0x00, 0x02, 0x00, 0x00 },
      { 0x40, 0x04, 0x01, 0x00, 0x00, 0x40, 0x08, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 7 (tier 2): (0,6),(0,17),(0,32),(0,45) | (1,3),(1,14),(1,33),(1,49) | (2,10),(2,24),(2,46),(2,59) */
    { { 0x02, 0x00, 0x40, 0x00, 0x80, 0x04, 0x00, 0x00 },
      { 0x10, 0x02, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00 },
      { 0x00, 0x20, 0x00, 0x80, 0x00, 0x02, 0x00, 0x10 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
};

LOCAL const uint8_t chunk_tier[CHUNK_COUNT] = { 0, 0, 0, 1, 1, 1, 2, 2 };

LOCAL void world_clear(struct game *g) {
    memset(g->world, 0, sizeof(g->world));
}

LOCAL char world_get(const struct game *g, int row, int col) {
    int idx = (g->world_top + row) % BUF_H;
    return g->world[idx][col];
}

LOCAL void world_set(struct game *g, int row, int col, char v) {
    int idx = (g->world_top + row) % BUF_H;
    g->world[idx][col] = v;
}

LOCAL int chunk_pick(unsigned int *rng, int distance) {
    /* Density ramp:
     *   distance <  100  -> tier 0 only
     *   distance <  200  -> tier 0 or 1 (50/50)
     *   distance >= 200  -> tier 1 or 2 (50/50)
     */
    int min_tier, max_tier;
    if (distance < 100)       { min_tier = 0; max_tier = 0; }
    else if (distance < 200)  { min_tier = 0; max_tier = 1; }
    else                      { min_tier = 1; max_tier = 2; }

    for (;;) {
        int i = (int)xorshift_uniform(rng, CHUNK_COUNT);
        if (chunk_tier[i] >= min_tier && chunk_tier[i] <= max_tier) return i;
    }
}

LOCAL void chunk_unpack(struct game *g, int dest_row, int chunk_idx) {
    for (int r = 0; r < CHUNK_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            int byte = c / 8;
            int bit  = 7 - (c % 8);
            int set  = (chunk_pool[chunk_idx][r][byte] >> bit) & 1;
            world_set(g, dest_row + r, c, set ? 'T' : 0);
        }
    }
}

LOCAL void world_scroll(struct game *g) {
    g->world_top = (g->world_top + 1) % BUF_H;
}

LOCAL int world_gen_chunk(struct game *g) {
    /* Write a fresh chunk into rows [BUF_H - CHUNK_ROWS .. BUF_H - 1]
     * relative to world_top. Returns dest_row. */
    int dest_row = BUF_H - CHUNK_ROWS;
    int pick = chunk_pick(&g->rng, g->distance);
    chunk_unpack(g, dest_row, pick);
    return dest_row;
}

/* --- player --- */
LOCAL int player_lean_input(int input) {
    return input == KEY_LEFT || input == KEY_RIGHT;
}

LOCAL void player_step(struct game *g, int input) {
    if (input == KEY_LEFT && g->player_col > 0) g->player_col--;
    else if (input == KEY_RIGHT && g->player_col < PLAYFIELD_W - 2) g->player_col++;
}

LOCAL void player_check_collision(struct game *g) {
    char left  = world_get(g, PLAYER_ROW, g->player_col);
    char right = world_get(g, PLAYER_ROW, g->player_col + 1);
    if (left == 'T' || right == 'T') g->alive = 0;
}

/* --- yeti --- */
/* (filled in Task 14) */

/* --- term --- */
/* (filled in Task 16) */

/* --- step / draw --- */
LOCAL void game_init(struct game *g, unsigned int seed) {
    memset(g, 0, sizeof(*g));
    g->rng = seed ? seed : 1;
    g->alive = 1;
    g->player_col = PLAYFIELD_W / 2 - 1;
    g->scroll_period = SCROLL_PERIOD_START;
    g->world_top = 0;
    world_clear(g);

    int range_s = REVEAL_MAX_S - REVEAL_MIN_S;
    int range_ticks = range_s * (1000 / FRAME_MS);
    g->yeti_reveal_tick = REVEAL_MIN_S * (1000 / FRAME_MS)
                        + (int)xorshift_uniform(&g->rng, (unsigned int)range_ticks);
}

/* --- cli --- */
/* (filled in Task 20) */

/* --- main --- */
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fputs("yeti: stub\n", stdout);
    return 0;
}
#endif
