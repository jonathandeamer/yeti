/* SPDX-License-Identifier: MIT OR CC0-1.0 */
#define _POSIX_C_SOURCE 200809L

/* --- includes --- */
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

/* --- player --- */
/* (filled in Task 13) */

/* --- yeti --- */
/* (filled in Task 14) */

/* --- term --- */
/* (filled in Task 16) */

/* --- step / draw --- */
/* (filled in Tasks 12, 15, 17, 18, 19) */

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
