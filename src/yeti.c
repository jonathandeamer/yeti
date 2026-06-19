/* SPDX-License-Identifier: MIT OR CC0-1.0 */
#define _POSIX_C_SOURCE 200809L

/* --- includes --- */
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* --- constants --- */
#define FRAME_MS         33
#define PLAYFIELD_W      60
#define PLAYFIELD_X_OFF  5
#define PLAYER_ROW       7
#define BUF_H            32
#define FP_SCALE         256
#define CHUNK_COUNT      8
#define CHUNK_ROWS       4
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
static void add_ms(struct timespec *t, long ms) {
    t->tv_sec += ms / 1000;
    t->tv_nsec += (ms % 1000) * 1000000L;
    if (t->tv_nsec >= 1000000000L) {
        t->tv_sec += 1;
        t->tv_nsec -= 1000000000L;
    }
}

static int ms_until(struct timespec t) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long sec = t.tv_sec - now.tv_sec;
    long nsec = t.tv_nsec - now.tv_nsec;
    return (int)(sec * 1000 + nsec / 1000000);
}

/* --- prng --- */
static unsigned int xorshift32(unsigned int *s) {
    unsigned int x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

static unsigned int xorshift_uniform(unsigned int *s, unsigned int n) {
    return xorshift32(s) % n;
}

/* --- world --- */
static const char *chunks[CHUNK_COUNT] = {
    "18,34|24,42|12,30,48",
    "10,28,46|20,38|15,33,51",
    "14,32,50|8,26,44|22,40",
    "5,19,38|12,29,46|8,21,45",
    "2,15,36|8,27,47|4,18,36",
    "7,22,41|2,20,45|11,31,48",
    "3,10,22,38,50|6,16,31,46|1,13,23,41,52",
    "6,17,32,45|3,14,33,49|10,24,46,59",
};

static const uint8_t chunk_tier[CHUNK_COUNT] = { 0, 0, 0, 1, 1, 1, 2, 2 };

static void world_clear(struct game *g) {
    memset(g->world, 0, sizeof(g->world));
}

static char world_get(const struct game *g, int row, int col) {
    int idx = (g->world_top + row) % BUF_H;
    return g->world[idx][col];
}

static void world_set(struct game *g, int row, int col, char v) {
    if (col >= 0 && col < PLAYFIELD_W) {
        int idx = (g->world_top + row) % BUF_H;
        g->world[idx][col] = v;
    }
}

static int chunk_pick(unsigned int *rng, int distance) {
    int min_tier, max_tier;
    if (distance < 40)        { min_tier = 0; max_tier = 0; }
    else if (distance < 120)  { min_tier = 0; max_tier = 1; }
    else                      { min_tier = 1; max_tier = 2; }

    for (;;) {
        int i = (int)xorshift_uniform(rng, CHUNK_COUNT);
        if (chunk_tier[i] >= min_tier && chunk_tier[i] <= max_tier) return i;
    }
}

static void chunk_unpack(struct game *g, int dest_row, int chunk_idx) {
    for (int r = 0; r < CHUNK_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            world_set(g, dest_row + r, c, 0);
        }
    }
    const char *s = chunks[chunk_idx];
    int row = 0, col = 0, have = 0;
    for (;; s++) {
        char ch = *s;
        if (ch >= '0' && ch <= '9') {
            col = col * 10 + (ch - '0');
            have = 1;
        } else {
            if (have) {
                world_set(g, dest_row + row, col, 'T');
                col = 0;
                have = 0;
            }
            if (ch == '|') {
                row++;
            } else if (ch == '\0') {
                break;
            }
        }
    }
}

static void world_scroll(struct game *g) {
    g->world_top = (g->world_top + 1) % BUF_H;
}

/* --- player --- */
static int player_lean_input(int input) {
    return input == KEY_LEFT || input == KEY_RIGHT;
}

static void player_step(struct game *g, int input) {
    if (input == KEY_LEFT && g->player_col > 0) g->player_col--;
    else if (input == KEY_RIGHT && g->player_col < PLAYFIELD_W - 2) g->player_col++;
}

static void player_check_collision(struct game *g) {
    char left  = world_get(g, PLAYER_ROW, g->player_col);
    char right = world_get(g, PLAYER_ROW, g->player_col + 1);
    if (left == 'T' || right == 'T') g->alive = 0;
}

/* --- yeti --- */
static void yeti_step(struct game *g, int input) {
    if (!g->yeti_armed) {
        if (g->tick >= g->yeti_reveal_tick) {
            g->yeti_armed = 1;
            g->yeti_row_fp = 0;
            g->yeti_col = PLAYFIELD_W / 2;
            if (g->scroll_period > SCROLL_PERIOD_END)
                g->scroll_period -= 1;
        }
        return;
    }

    g->yeti_row_fp += BASE_CLOSING_FP;
    if (player_lean_input(input)) g->yeti_row_fp += PRESS_COST_FP;

    if (g->yeti_col < g->player_col) g->yeti_col++;
    else if (g->yeti_col > g->player_col) g->yeti_col--;

    if (g->yeti_row_fp >= PLAYER_ROW * FP_SCALE) g->alive = 0;
}

/* --- term --- */
static volatile sig_atomic_t sig_quit = 0;

static void on_signal(int sig) {
    (void)sig;
    sig_quit = 1;
}

static void term_cleanup(void) {
    endwin();
}

static int term_init(void) {
    setenv("ESCDELAY", "25", 0);
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(PAIR_TREE,   COLOR_GREEN,  COLOR_BLACK);
    init_pair(PAIR_PLAYER, COLOR_CYAN,   COLOR_BLACK);
    init_pair(PAIR_YETI,   COLOR_RED,    COLOR_BLACK);
    init_pair(PAIR_HUD,    COLOR_WHITE,  COLOR_BLACK);

    if (LINES < 24 || COLS < 80) {
        endwin();
        fputs("needs 80x24\n", stderr);
        return 0;
    }

    struct sigaction sa = { .sa_handler = on_signal };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    return 1;
}

/* --- step / draw --- */
static void game_init(struct game *g, unsigned int seed) {
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

static void update_scroll_period(struct game *g) {
    int ramp_ticks = 60 * (1000 / FRAME_MS);
    int span = SCROLL_PERIOD_START - SCROLL_PERIOD_END;
    int reduced = (g->tick * span) / ramp_ticks;
    if (reduced > span) reduced = span;
    int target = SCROLL_PERIOD_START - reduced;
    if (target < g->scroll_period) g->scroll_period = target;
}

static void step(struct game *g, int input) {
    if (!g->alive) return;

    g->tick++;
    update_scroll_period(g);

    player_step(g, input);
    yeti_step(g, input);

    g->ticks_since_scroll++;
    if (g->ticks_since_scroll >= g->scroll_period) {
        g->ticks_since_scroll = 0;
        world_scroll(g);
        g->distance++;
        if ((g->distance % CHUNK_ROWS) == 0) {
            chunk_unpack(g, BUF_H - CHUNK_ROWS, chunk_pick(&g->rng, g->distance));
        }
    }

    player_check_collision(g);
}

static void draw(const struct game *g) {
    erase();

    for (int r = 0; r < LINES && r < 24; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            char cell = world_get(g, r, c);
            if (cell == 'T') {
                attron(COLOR_PAIR(PAIR_TREE));
                mvaddch(r, PLAYFIELD_X_OFF + c, 'T');
                attroff(COLOR_PAIR(PAIR_TREE));
            }
        }
    }

    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + g->player_col, '|');
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + g->player_col + 1, '|');
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);

    if (g->yeti_armed) {
        int yrow = g->yeti_row_fp / FP_SCALE;
        if (yrow >= 0 && yrow < LINES) {
            attron(COLOR_PAIR(PAIR_YETI) | A_BOLD);
            mvaddch(yrow, PLAYFIELD_X_OFF + g->yeti_col, 'Y');
            attroff(COLOR_PAIR(PAIR_YETI) | A_BOLD);
        }
    }

    attron(COLOR_PAIR(PAIR_HUD));
    mvprintw(0, COLS - 13, "DIST %5dm", g->distance);
    attroff(COLOR_PAIR(PAIR_HUD));

    refresh();
}

static int death_screen(const struct game *g) {
    int cx = PLAYFIELD_X_OFF + PLAYFIELD_W / 2, cy = 12;
redraw:
    draw(g);
    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvprintw(cy - 1, cx - 8, "YOU SKIED %5dm", g->distance);
    mvprintw(cy + 1, cx - 16, "PRESS R TO RESTART  -  Q TO QUIT");
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    refresh();

    timeout(200);
    for (;;) {
        int ch = getch();
        if (sig_quit) return 0;
        if (ch == 'r' || ch == 'R') return 1;
        if (ch == 'q' || ch == 'Q' || ch == 27) return 0;
        if (ch == KEY_RESIZE) goto redraw;
    }
}

static void play_one_run(struct game *g) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    chunk_unpack(g, BUF_H - CHUNK_ROWS * 2, chunk_pick(&g->rng, 0));
    chunk_unpack(g, BUF_H - CHUNK_ROWS,     chunk_pick(&g->rng, 0));

    while (g->alive && !sig_quit) {
        add_ms(&next, FRAME_MS);
        int input = 0;
        for (;;) {
            int ms_left = ms_until(next);
            timeout(ms_left > 0 ? ms_left : 0);
            int ch = getch();
            if (ch == ERR) break;
            input = ch;
            if (ch == 'q' || ch == 'Q' || ch == 27) { sig_quit = 1; break; }
        }
        step(g, input);
        draw(g);
    }
}

/* --- main --- */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (!term_init()) return 1;

    struct game g;
    game_init(&g, (unsigned int)(time(NULL) ^ getpid()));

    while (!sig_quit) {
        play_one_run(&g);
        if (sig_quit || g.alive) break;
        if (!death_screen(&g)) break;
        game_init(&g, (unsigned int)(time(NULL) ^ getpid() ^ g.rng));
    }

    term_cleanup();
    return 0;
}
