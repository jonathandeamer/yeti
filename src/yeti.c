/* SPDX-License-Identifier: MIT OR CC0-1.0 */
#define _POSIX_C_SOURCE 200809L

/* --- includes --- */
#include <stdio.h>

/* --- test visibility --- */
#ifdef GAME_TEST
#define LOCAL
#else
#define LOCAL static
#endif

/* --- constants --- */
/* (filled in Task 7) */

/* --- state --- */
/* (filled in Task 7) */

/* --- time --- */
/* (filled in Task 6) */

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
/* (filled in Tasks 8-11) */

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
