# yeti — byte-budget reduction implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring `wc -c src/yeti.c` from 15,854 down to ≤ 10,240 bytes while preserving all player-visible behavior.

**Architecture:** Eight phases, each one commit, applied in increasing order of pain. Stop the moment `make size` reports ≤ 10240. Tests stay green through phases 1–7; the test suite is dropped in phase 8.

**Tech Stack:** Same as before — C99 + X/Open Curses (ncurses), POSIX-2008. macOS-only after this work (Linux/BSD support can be restored separately).

**Cross-references:**
- Spec: `docs/superpowers/specs/2026-05-25-yeti-byte-budget-design.md` — every decision below is sourced from this file.
- Current source: `src/yeti.c` at 15,854 bytes (per `make size`).

---

## File structure

**Modified by every phase:**
- `src/yeti.c` — the only source file. All cuts land here.

**Modified by phases 2 and 8:**
- `tests/test_yeti.c` — phase 2 drops CLI-related tests; phase 8 deletes the file entirely.
- `Makefile` — phase 8 removes the `test` and `tests/test_yeti` targets.

**Not modified:**
- `docs/superpowers/specs/` and `docs/superpowers/plans/` — design docs stay.
- `FORBIDDEN-SOURCES.md`, `README.md`, `LICENSE`, `LICENSES/*.txt` — repo hygiene unchanged.

---

## Commit-message conventions

Phases are byte-reduction work. Best-fit type: `refactor` when restructuring code, `perf` when the purpose is explicitly byte savings. Scope follows the area of code being cut.

Examples for each phase:
- Phase 1: `refactor(world): drop chunk-pool position comments and verbose strings`
- Phase 2: `refactor(cli): drop CLI section entirely; seed always from time^pid`
- Phase 3: `refactor(term): macOS-only term cleanup`
- Phase 4: `perf(world): re-encode chunk pool as position-list strings`
- Phase 5: `refactor(game): inline trivial wrappers at single call sites`
- Phase 6 (if applied): `refactor(game): promote struct game to file-scope globals`
- Phase 7 (if applied): `refactor(game): code-golf hot paths`
- Phase 8: `refactor(test): drop test scaffolding and tests/test_yeti.c`

---

### Task 1: Phase 1 — Easy wins (no behavior change)

**Files:**
- Modify: `src/yeti.c`

**Goal:** Strip dead/decorative bytes. Target saving: ~1,000 bytes. Landing: ~14,850.

- [ ] **Step 1: Delete the `_POSIX_C_SOURCE` define**

In `src/yeti.c`, replace:

```c
/* SPDX-License-Identifier: MIT OR CC0-1.0 */
#define _POSIX_C_SOURCE 200809L

/* --- includes --- */
```

with:

```c
/* SPDX-License-Identifier: MIT OR CC0-1.0 */

/* --- includes --- */
```

- [ ] **Step 2: Strip the chunk-pool position comments**

In `src/yeti.c`, replace the entire `chunk_pool` declaration (lines 93–138 today) with this comment-free version:

```c
/* --- world --- */
LOCAL const uint8_t chunk_pool[CHUNK_COUNT][CHUNK_ROWS][CHUNK_BYTES] = {
    { { 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00 },
      { 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x40, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 },
      { 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x04, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00 },
      { 0x00, 0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00 },
      { 0x00, 0x80, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x20, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 },
      { 0x00, 0x80, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00 },
      { 0x08, 0x00, 0x20, 0x00, 0x08, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x01, 0x00, 0x02, 0x00, 0x00, 0x40, 0x00, 0x00 },
      { 0x20, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x10, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x10, 0x20, 0x02, 0x00, 0x02, 0x00, 0x20, 0x00 },
      { 0x02, 0x00, 0x80, 0x01, 0x00, 0x02, 0x00, 0x00 },
      { 0x40, 0x04, 0x01, 0x00, 0x00, 0x40, 0x08, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { { 0x02, 0x00, 0x40, 0x00, 0x80, 0x04, 0x00, 0x00 },
      { 0x10, 0x02, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00 },
      { 0x00, 0x20, 0x00, 0x80, 0x00, 0x02, 0x00, 0x10 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
};
```

- [ ] **Step 3: Trim error strings**

In `src/yeti.c`, replace:

```c
        fputs("yeti: requires a real terminal (TERM unset or dumb)\n", stderr);
```

with:

```c
        fputs("needs a real terminal\n", stderr);
```

Replace:

```c
        fputs("yeti: initscr failed\n", stderr);
```

with:

```c
        fputs("initscr failed\n", stderr);
```

Replace:

```c
        fputs("yeti: requires an 80x24 terminal or larger\n", stderr);
```

with:

```c
        fputs("needs 80x24\n", stderr);
```

Replace:

```c
            fprintf(stderr, "yeti: unknown argument: %s\n", a);
```

with:

```c
            fprintf(stderr, "unknown: %s\n", a);
```

- [ ] **Step 4: Strip in-function explanatory comments**

Remove the comment lines inside function bodies. Section dividers (`/* --- name --- */`) stay. Specifically:

In `xorshift_uniform`, delete:

```c
    /* Slight modulo bias acceptable for game variance use. */
```

In `chunk_pool` header (already addressed in Step 2; the `/* Bit layout: byte B... */` line above the declaration goes too):

```c
/* Bit layout: byte B covers cols B*8..B*8+7; bit 7 = leftmost (col B*8). */
```

In `chunk_pick`, delete:

```c
    /* Density ramp:
     *   distance <  100  -> tier 0 only
     *   distance <  200  -> tier 0 or 1 (50/50)
     *   distance >= 200  -> tier 1 or 2 (50/50)
     */
```

In `world_gen_chunk`, delete:

```c
    /* Write a fresh chunk into rows [BUF_H - CHUNK_ROWS .. BUF_H - 1]
     * relative to world_top. Returns dest_row. */
```

In `update_scroll_period`, delete:

```c
    /* Linear ramp from SCROLL_PERIOD_START to SCROLL_PERIOD_END over 60 s. */
```

And:

```c
    /* Allow the yeti-reveal jolt (which already nudged it lower) to stick. */
```

In `draw`, delete:

```c
    /* Playfield rows */
```

and

```c
    /* Player (two cells) */
```

and

```c
    /* Yeti */
```

and

```c
    /* HUD */
```

In `death_screen`, delete:

```c
    /* Returns 1 if player wants to restart, 0 to quit. */
    ...
    /* Short timeout so we wake up to poll sig_quit even if libc/ncurses
     * auto-restart the underlying read across a signal. */
```

In `play_one_run`, delete:

```c
    /* max_ticks = 0 means run until death or quit. */
    ...
    /* Seed the world with two chunks so something is on screen at start. */
```

In `main`, delete:

```c
    if (opts.ticks > 0) break;  /* test mode: one run then exit */
```

(Keep the `if` statement; just drop the trailing comment.)

- [ ] **Step 5: Build and test**

Run:
```bash
make clean && make && make test && make size
```

Expected: builds; 3572/3572 tests pass; `make size` reports somewhere around 14,850 bytes (give or take ±200).

- [ ] **Step 6: Commit**

```bash
git add src/yeti.c
git commit -m "$(cat <<'EOF'
refactor(world): drop position comments, verbose strings, and POSIX macro

Phase 1 of the byte-budget reduction. Strips the eight position-list
comments inside chunk_pool, the /* Bit layout */ header, the
in-function explanatory comments throughout, and shortens the four
error strings. Drops the _POSIX_C_SOURCE define (macOS exposes the
symbols without it).

No behavior change. Tests still 3572/3572.
EOF
)"
```

---

### Task 2: Phase 2 — Drop CLI entirely

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

**Goal:** Remove the entire CLI section and its plumbing. Target saving: ~1,311 bytes. Landing: ~13,540.

- [ ] **Step 1: Delete the CLI section from `src/yeti.c`**

Remove the entire block from `/* --- cli --- */` through the closing `}` of `cli_print_help`. After deletion the next section divider after `/* --- term --- */` (and the step/draw block) should be `/* --- main --- */`.

The specific text to delete:

```c
/* --- cli --- */
struct cli_opts {
    int show_help;
    int have_seed;
    unsigned int seed;
    int ticks;
};

LOCAL int cli_parse(int argc, char **argv, struct cli_opts *opts) {
    memset(opts, 0, sizeof(*opts));
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-h") || !strcmp(a, "--help")) {
            opts->show_help = 1;
        } else if (!strcmp(a, "-s") && i + 1 < argc) {
            opts->have_seed = 1;
            opts->seed = (unsigned int)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(a, "--ticks") && i + 1 < argc) {
            opts->ticks = (int)strtol(argv[++i], NULL, 10);
        } else {
            fprintf(stderr, "unknown: %s\n", a);
            return 1;
        }
    }
    return 0;
}

LOCAL void cli_print_help(void) {
    fputs(
        "yeti - ski down a mountain until the yeti finds you\n"
        "\n"
        "USAGE\n"
        "    yeti [-s SEED]\n"
        "\n"
        "OPTIONS\n"
        "    -h, --help        show this help and exit\n"
        "    -s SEED           set RNG seed for a deterministic run\n"
        "\n"
        "CONTROLS\n"
        "    LEFT / RIGHT      lean\n"
        "    Q, ESC            quit\n"
        "    R                 restart from death\n",
        stdout);
}

```

- [ ] **Step 2: Drop the `max_ticks` parameter from `play_one_run`**

Replace the entire `play_one_run` function:

```c
LOCAL void play_one_run(struct game *g) {
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
        if (sig_resize) {
            sig_resize = 0;
            if (LINES < 24 || COLS < 80) { sig_quit = 1; break; }
        }
        step(g, input);
        draw(g);
    }
}
```

- [ ] **Step 3: Simplify `main`**

Replace the entire `main` function:

```c
#ifndef GAME_TEST
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
        game_init(&g, (unsigned int)(time(NULL) ^ getpid()));
    }

    term_cleanup();
    return 0;
}
#endif
```

- [ ] **Step 4: Remove CLI tests from `tests/test_yeti.c`**

Delete these test functions:

```c
static void test_cli_parse_no_args(void) {
    char *argv[] = { (char *)"yeti", NULL };
    struct cli_opts opts;
    int rc = cli_parse(1, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.have_seed == 0);
    ASSERT(opts.ticks == 0);
    ASSERT(opts.show_help == 0);
}

static void test_cli_parse_seed(void) {
    char *argv[] = { (char *)"yeti", (char *)"-s", (char *)"12345", NULL };
    struct cli_opts opts;
    int rc = cli_parse(3, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.have_seed == 1);
    ASSERT(opts.seed == 12345);
}

static void test_cli_parse_ticks(void) {
    char *argv[] = { (char *)"yeti", (char *)"--ticks", (char *)"100", NULL };
    struct cli_opts opts;
    int rc = cli_parse(3, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.ticks == 100);
}

static void test_cli_parse_help(void) {
    char *argv[] = { (char *)"yeti", (char *)"-h", NULL };
    struct cli_opts opts;
    int rc = cli_parse(2, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.show_help == 1);
}

static void test_cli_parse_unknown_returns_error(void) {
    char *argv[] = { (char *)"yeti", (char *)"--nope", NULL };
    struct cli_opts opts;
    int rc = cli_parse(2, argv, &opts);
    ASSERT(rc != 0);
}
```

Also remove the corresponding `test_cli_*()` calls from `main()` in `tests/test_yeti.c`:

```c
    test_cli_parse_no_args();
    test_cli_parse_seed();
    test_cli_parse_ticks();
    test_cli_parse_help();
    test_cli_parse_unknown_returns_error();
```

- [ ] **Step 5: Build and test**

Run:
```bash
make clean && make && make test && make size
```

Expected: builds; 3567/3567 tests pass (5 CLI tests removed); `make size` reports ~13,540 ± 200.

- [ ] **Step 6: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "$(cat <<'EOF'
refactor(cli): drop CLI section entirely; seed always from time^pid

Phase 2 of the byte-budget reduction. Removes struct cli_opts,
cli_parse, cli_print_help, and all four flag plumbing paths
(-h/--help, -s SEED, --ticks N). main ignores argv. play_one_run
loses its max_ticks parameter. Documentation lives in README only.

Companion tests for cli_parse are removed.

No player-visible behavior change beyond the flags themselves
disappearing. Tests 3567/3567.
EOF
)"
```

---

### Task 3: Phase 3 — macOS-only term cleanup

**Files:**
- Modify: `src/yeti.c`

**Goal:** Strip defensive curses init code that macOS doesn't need. Target saving: ~500 bytes. Landing: ~13,040.

- [ ] **Step 1: Replace the entire `/* --- term --- */` section**

Replace the block from `/* --- term --- */` through the closing `}` of `term_init` (and the now-empty `term_cleanup`) with this simpler version:

```c
/* --- term --- */
static volatile sig_atomic_t sig_quit = 0;
static volatile sig_atomic_t sig_resize = 0;

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
    init_pair(PAIR_PLAYER, COLOR_WHITE,  COLOR_BLACK);
    init_pair(PAIR_YETI,   COLOR_RED,    COLOR_BLACK);
    init_pair(PAIR_HUD,    COLOR_YELLOW, COLOR_BLACK);

    if (LINES < 24 || COLS < 80) {
        endwin();
        fputs("needs 80x24\n", stderr);
        return 0;
    }

    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);
    return 1;
}
```

Note: `sig_resize` is kept as a declaration only because `play_one_run` references it (the variable is now write-never, read-once-and-discard, but removing all references is part of step 2).

- [ ] **Step 2: Remove the `sig_resize` references in `play_one_run`**

Replace the `play_one_run` body's resize block:

```c
        if (sig_resize) {
            sig_resize = 0;
            if (LINES < 24 || COLS < 80) { sig_quit = 1; break; }
        }
```

with nothing (delete those four lines).

Then delete the now-unused declaration:

```c
static volatile sig_atomic_t sig_resize = 0;
```

- [ ] **Step 3: Build and test**

```bash
make clean && make && make test && make size
```

Expected: builds (warnings tolerated); 3567/3567 tests pass; `make size` reports ~13,040 ± 200.

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c
git commit -m "$(cat <<'EOF'
refactor(term): macOS-only term cleanup

Phase 3 of the byte-budget reduction. macOS-only target lets us
drop a stack of defensive code that the spec requires for Linux/BSD
portability:

- sigaction block -> three signal() calls (BSD-persistent on macOS)
- SIGWINCH handler and sig_resize flag removed
- atexit(term_cleanup) belt-and-suspenders removed
- has_colors() / use_default_colors() guards removed
- TERM unset/empty/dumb check removed
- initscr() null-return check removed
- curses_initialized flag removed

Background colors switch from -1 (default) to COLOR_BLACK, which
behaves the same on macOS Terminal. Mid-run resize check in
play_one_run also removed.
EOF
)"
```

---

### Task 4: Phase 4 — Re-encode chunks as position-list strings

**Files:**
- Modify: `src/yeti.c`

**Goal:** Replace the hex-byte grid with compact position-list strings + a small decoder. Target saving: ~1,400 bytes. Landing: ~11,640.

- [ ] **Step 1: Replace `chunk_pool` with the string table**

In `src/yeti.c`, replace the entire `chunk_pool` declaration (the eight 4×8 hex grids) with:

```c
LOCAL const char *chunks[CHUNK_COUNT] = {
    "27|12,45|40",
    "7,45|25|18,49",
    "18|6,37|26",
    "5,19,38|12,29,46|8,21,45",
    "2,15,36|8,27,47|4,18,36",
    "7,22,41|2,20,45|11,31,48",
    "3,10,22,38,50|6,16,31,46|1,13,23,41,52",
    "6,17,32,45|3,14,33,49|10,24,46,59",
};
```

The format: three pipe-separated rows. Each row is a comma-separated list of column positions (0–59). The fourth row of every chunk is implicit and empty.

- [ ] **Step 2: Replace `chunk_unpack` with a string-decoding version**

Replace the existing `chunk_unpack`:

```c
LOCAL void chunk_unpack(struct game *g, int dest_row, int chunk_idx) {
    /* Clear the four destination rows. */
    for (int r = 0; r < CHUNK_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            world_set(g, dest_row + r, c, 0);
        }
    }
    /* Walk the string: digits accumulate a column number; ',' commits
     * the column to the current row; '|' advances to the next row;
     * end-of-string flushes the last column. */
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
```

- [ ] **Step 3: Inline `world_gen_chunk` at its single call site in `step`**

Delete the `world_gen_chunk` function:

```c
LOCAL int world_gen_chunk(struct game *g) {
    int dest_row = BUF_H - CHUNK_ROWS;
    int pick = chunk_pick(&g->rng, g->distance);
    chunk_unpack(g, dest_row, pick);
    return dest_row;
}
```

And in `step`, replace:

```c
        if ((g->distance % CHUNK_ROWS) == 0) {
            world_gen_chunk(g);
        }
```

with:

```c
        if ((g->distance % CHUNK_ROWS) == 0) {
            chunk_unpack(g, BUF_H - CHUNK_ROWS, chunk_pick(&g->rng, g->distance));
        }
```

- [ ] **Step 4: Update tests/test_yeti.c — chunk-bit test helper and invariants**

The `chunk_bit` helper and the four invariant tests reference `chunk_pool[chunk][row][byte]` which no longer exists. Replace them with a string-based equivalent.

Replace in `tests/test_yeti.c`:

```c
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
    for (int chunk = 0; chunk < CHUNK_COUNT; chunk++) {
        for (int row = 0; row < CHUNK_ROWS; row++) {
            ASSERT((chunk_pool[chunk][row][CHUNK_BYTES - 1] & 0x0F) == 0);
        }
    }
}
```

with:

```c
/* Helper: render a chunk's row as a 60-element bool grid by unpacking
 * into a temporary struct game and reading via world_get. */
static void chunk_render_grid(int chunk_idx, int grid[CHUNK_ROWS][PLAYFIELD_W]) {
    struct game g;
    memset(&g, 0, sizeof g);
    g.world_top = 0;
    chunk_unpack(&g, 0, chunk_idx);
    for (int r = 0; r < CHUNK_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            grid[r][c] = (world_get(&g, r, c) == 'T') ? 1 : 0;
        }
    }
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
    int grid[CHUNK_ROWS][PLAYFIELD_W];
    for (int i = 0; i < CHUNK_COUNT; i++) {
        chunk_render_grid(i, grid);
        for (int c = 0; c < PLAYFIELD_W; c++) {
            ASSERT(grid[CHUNK_ROWS - 1][c] == 0);
        }
    }
}

static void test_chunk_no_three_consecutive_trees(void) {
    int grid[CHUNK_ROWS][PLAYFIELD_W];
    for (int chunk = 0; chunk < CHUNK_COUNT; chunk++) {
        chunk_render_grid(chunk, grid);
        for (int row = 0; row < CHUNK_ROWS; row++) {
            int run = 0;
            for (int col = 0; col < PLAYFIELD_W; col++) {
                if (grid[row][col]) {
                    run++;
                    ASSERT(run < 3);
                } else {
                    run = 0;
                }
            }
        }
    }
}

/* test_chunk_unused_bits_are_zero is dropped: position-list encoding
 * has no concept of unused tail bits. */
```

Also remove the now-stale call from `main()` in `tests/test_yeti.c`:

```c
    test_chunk_unused_bits_are_zero();
```

- [ ] **Step 5: Build and test**

```bash
make clean && make && make test && make size
```

Expected: builds; 3566/3566 tests pass (one chunk-invariant test dropped); `make size` reports ~11,640 ± 300.

- [ ] **Step 6: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "$(cat <<'EOF'
perf(world): re-encode chunk pool as position-list strings

Phase 4 of the byte-budget reduction. The 8x4x8 hex-byte grid
becomes 8 short strings like "27|12,45|40" — pipe-separated rows,
comma-separated column positions, fourth row implicit and empty.
chunk_unpack walks the string in one pass writing 'T' at each
position, clearing the four destination rows first.

world_gen_chunk inlined at its single call site in step. Tests
that read chunk_pool[][][] directly are rewritten against the
public interface (unpack into a temp game and read via world_get).
The unused-tail-bits invariant is dropped — string encoding has
no equivalent.
EOF
)"
```

---

### Task 5: Phase 5 — Inline trivial wrappers

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

**Goal:** Remove one-line/two-line helper functions whose call sites can absorb their bodies. Target saving: ~300 bytes. Landing: ~11,340.

- [ ] **Step 1: Inline `player_lean_input` and delete the helper**

Delete the entire function:

```c
LOCAL int player_lean_input(int input) {
    return input == KEY_LEFT || input == KEY_RIGHT;
}
```

In `yeti_step`, replace:

```c
    if (player_lean_input(input)) g->yeti_row_fp += PRESS_COST_FP;
```

with:

```c
    if (input == KEY_LEFT || input == KEY_RIGHT) g->yeti_row_fp += PRESS_COST_FP;
```

- [ ] **Step 2: Inline `update_scroll_period` and delete the helper**

Delete the entire function:

```c
LOCAL void update_scroll_period(struct game *g) {
    int ramp_ticks = 60 * (1000 / FRAME_MS);
    int span = SCROLL_PERIOD_START - SCROLL_PERIOD_END;
    int reduced = (g->tick * span) / ramp_ticks;
    if (reduced > span) reduced = span;
    int target = SCROLL_PERIOD_START - reduced;
    if (target < g->scroll_period) g->scroll_period = target;
}
```

In `step`, replace:

```c
    g->tick++;
    update_scroll_period(g);
```

with:

```c
    g->tick++;
    {
        int span = SCROLL_PERIOD_START - SCROLL_PERIOD_END;
        int reduced = (g->tick * span) / (60 * (1000 / FRAME_MS));
        if (reduced > span) reduced = span;
        int target = SCROLL_PERIOD_START - reduced;
        if (target < g->scroll_period) g->scroll_period = target;
    }
```

- [ ] **Step 3: Inline `xorshift_uniform` and delete the helper**

Delete the entire function:

```c
LOCAL unsigned int xorshift_uniform(unsigned int *s, unsigned int n) {
    return xorshift32(s) % n;
}
```

At every call site, replace `xorshift_uniform(&rng_ptr, n)` with `xorshift32(&rng_ptr) % n`.

In `chunk_pick`:
```c
        int i = (int)xorshift_uniform(rng, CHUNK_COUNT);
```
becomes:
```c
        int i = (int)(xorshift32(rng) % CHUNK_COUNT);
```

In `game_init`:
```c
    g->yeti_reveal_tick = REVEAL_MIN_S * (1000 / FRAME_MS)
                        + (int)xorshift_uniform(&g->rng, (unsigned int)range_ticks);
```
becomes:
```c
    g->yeti_reveal_tick = REVEAL_MIN_S * (1000 / FRAME_MS)
                        + (int)(xorshift32(&g->rng) % (unsigned int)range_ticks);
```

- [ ] **Step 4: Inline `add_ms` and `ms_until` (used only in `play_one_run`)**

Delete:

```c
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
```

In `play_one_run`, replace:

```c
        add_ms(&next, FRAME_MS);
        int input = 0;
        for (;;) {
            int ms_left = ms_until(next);
            timeout(ms_left > 0 ? ms_left : 0);
```

with:

```c
        next.tv_nsec += FRAME_MS * 1000000L;
        if (next.tv_nsec >= 1000000000L) { next.tv_sec += 1; next.tv_nsec -= 1000000000L; }
        int input = 0;
        for (;;) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            long sec = next.tv_sec - now.tv_sec;
            long nsec = next.tv_nsec - now.tv_nsec;
            int ms_left = (int)(sec * 1000 + nsec / 1000000);
            timeout(ms_left > 0 ? ms_left : 0);
```

Note: `add_ms`'s `t->tv_sec += ms / 1000` is dropped because `FRAME_MS` (33) is always < 1000, so `ms / 1000` is always 0 at our call site.

- [ ] **Step 5: Inline `world_clear` in `game_init`**

Delete:

```c
LOCAL void world_clear(struct game *g) {
    memset(g->world, 0, sizeof(g->world));
}
```

In `game_init`, replace:

```c
    g->world_top = 0;
    world_clear(g);
```

with:

```c
    g->world_top = 0;
    memset(g->world, 0, sizeof(g->world));
```

- [ ] **Step 6: Remove now-broken tests from `tests/test_yeti.c`**

The following tests reference helpers that no longer exist. Delete them:

```c
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
```

Also remove their calls from `main()` in `tests/test_yeti.c`:

```c
    test_xorshift_uniform_in_range();
    test_add_ms_basic();
    test_add_ms_wrap();
    test_ms_until_positive();
    test_ms_until_past_returns_nonpositive();
    test_world_clear_zeros_buffer();
```

- [ ] **Step 7: Build and test**

```bash
make clean && make && make test && make size
```

Expected: builds; ~2,560 tests pass (about a thousand from the dropped xorshift_uniform loop are gone); `make size` reports ~11,340 ± 200.

- [ ] **Step 8: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "$(cat <<'EOF'
refactor(game): inline trivial wrappers at single call sites

Phase 5 of the byte-budget reduction. Six helpers absorbed into
their callers:

- player_lean_input -> inline check in yeti_step
- update_scroll_period -> inline block in step
- xorshift_uniform -> xorshift32(&s) % n at each call site
- add_ms -> two-line tv_nsec += FRAME_MS*1e6L; carry check
- ms_until -> inline clock_gettime + arithmetic
- world_clear -> memset in game_init

Companion unit tests for the deleted helpers go too. Player-visible
behavior unchanged.
EOF
)"
```

---

### Task 6: 🛑 Checkpoint — measure and decide

**Files:** (no edits)

- [ ] **Step 1: Measure**

Run:
```bash
make clean && make && make size
```

Note the byte count. Expected: somewhere between 10,800 and 11,500.

- [ ] **Step 2: Decide which conditional phases to run**

| `make size` reports | Action |
|---|---|
| ≤ 10,240 | Skip Tasks 7 and 8 (phases 6 and 7). Jump straight to Task 9 (phase 8). |
| 10,241 – 10,950 | Run only Task 8 (phase 7: light code-golf). Skip Task 7. |
| 10,951 – 11,800 | Run Task 7 (phase 6: globals refactor). Re-measure. If still > 10,240, also run Task 8. |
| > 11,800 | Something went off-plan. Pause and reassess before running more phases. |

This task is decision-only — no commit, no file changes.

---

### Task 7: Phase 6 — Promote `struct game` to file-scope globals (CONDITIONAL)

**Run only if Task 6 says so.**

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

**Goal:** Remove the `struct game` indirection. Target saving: ~600 bytes. Landing: ~10,740.

- [ ] **Step 1: Replace the state section with globals**

In `src/yeti.c`, replace:

```c
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
```

with:

```c
/* --- state --- */
LOCAL unsigned int rng;
LOCAL int tick;
LOCAL int distance;
LOCAL int player_col;
LOCAL int alive;
LOCAL int yeti_armed;
LOCAL int yeti_reveal_tick;
LOCAL int yeti_row_fp;
LOCAL int yeti_col;
LOCAL int scroll_period;
LOCAL int ticks_since_scroll;
LOCAL char world[BUF_H][PLAYFIELD_W];
LOCAL int world_top;
```

- [ ] **Step 2: Rewrite every function signature and body**

This is a mechanical sweep through `src/yeti.c`:

- Every function that took `struct game *g` (or `const struct game *g`) loses that parameter.
- Every function that took `struct game *g, int X` becomes `int X`.
- Every `g->FIELD` becomes `FIELD`.
- Every call site loses `&g, ` from its argument list.
- `game_init` takes a `unsigned int seed` only.
- `play_one_run` takes no args.

Below is the full rewritten contents of every changed function. Replace each in turn.

```c
LOCAL char world_get(int row, int col) {
    int idx = (world_top + row) % BUF_H;
    return world[idx][col];
}

LOCAL void world_set(int row, int col, char v) {
    int idx = (world_top + row) % BUF_H;
    world[idx][col] = v;
}

LOCAL int chunk_pick(int dist) {
    int min_tier, max_tier;
    if (dist < 100)       { min_tier = 0; max_tier = 0; }
    else if (dist < 200)  { min_tier = 0; max_tier = 1; }
    else                  { min_tier = 1; max_tier = 2; }

    for (;;) {
        int i = (int)(xorshift32(&rng) % CHUNK_COUNT);
        if (chunk_tier[i] >= min_tier && chunk_tier[i] <= max_tier) return i;
    }
}

LOCAL void chunk_unpack(int dest_row, int chunk_idx) {
    for (int r = 0; r < CHUNK_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            world_set(dest_row + r, c, 0);
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
                world_set(row, col, 'T');
                col = 0;
                have = 0;
            }
            if (ch == '|') row++;
            else if (ch == '\0') break;
        }
    }
}

LOCAL void world_scroll(void) {
    world_top = (world_top + 1) % BUF_H;
}

LOCAL void player_step(int input) {
    if (input == KEY_LEFT && player_col > 0) player_col--;
    else if (input == KEY_RIGHT && player_col < PLAYFIELD_W - 2) player_col++;
}

LOCAL void player_check_collision(void) {
    if (world_get(PLAYER_ROW, player_col) == 'T' ||
        world_get(PLAYER_ROW, player_col + 1) == 'T') alive = 0;
}

LOCAL void yeti_step(int input) {
    if (!yeti_armed) {
        if (tick >= yeti_reveal_tick) {
            yeti_armed = 1;
            yeti_row_fp = 0;
            yeti_col = PLAYFIELD_W / 2;
            if (scroll_period > SCROLL_PERIOD_END) scroll_period--;
        }
        return;
    }
    yeti_row_fp += BASE_CLOSING_FP;
    if (input == KEY_LEFT || input == KEY_RIGHT) yeti_row_fp += PRESS_COST_FP;
    if (yeti_col < player_col) yeti_col++;
    else if (yeti_col > player_col) yeti_col--;
    if (yeti_row_fp >= PLAYER_ROW * FP_SCALE) alive = 0;
}

LOCAL void game_init(unsigned int seed) {
    rng = seed ? seed : 1;
    tick = 0;
    distance = 0;
    alive = 1;
    yeti_armed = 0;
    yeti_row_fp = 0;
    yeti_col = 0;
    scroll_period = SCROLL_PERIOD_START;
    ticks_since_scroll = 0;
    player_col = PLAYFIELD_W / 2 - 1;
    world_top = 0;
    memset(world, 0, sizeof(world));

    int range_s = REVEAL_MAX_S - REVEAL_MIN_S;
    int range_ticks = range_s * (1000 / FRAME_MS);
    yeti_reveal_tick = REVEAL_MIN_S * (1000 / FRAME_MS)
                     + (int)(xorshift32(&rng) % (unsigned int)range_ticks);
}

LOCAL void step(int input) {
    if (!alive) return;

    tick++;
    {
        int span = SCROLL_PERIOD_START - SCROLL_PERIOD_END;
        int reduced = (tick * span) / (60 * (1000 / FRAME_MS));
        if (reduced > span) reduced = span;
        int target = SCROLL_PERIOD_START - reduced;
        if (target < scroll_period) scroll_period = target;
    }

    player_step(input);
    yeti_step(input);

    ticks_since_scroll++;
    if (ticks_since_scroll >= scroll_period) {
        ticks_since_scroll = 0;
        world_scroll();
        distance++;
        if ((distance % CHUNK_ROWS) == 0) {
            chunk_unpack(BUF_H - CHUNK_ROWS, chunk_pick(distance));
        }
    }

    player_check_collision();
}

LOCAL void draw(void) {
    erase();

    for (int r = 0; r < LINES && r < BUF_H; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            if (world_get(r, c) == 'T') {
                attron(COLOR_PAIR(PAIR_TREE));
                mvaddch(r, PLAYFIELD_X_OFF + c, 'T');
                attroff(COLOR_PAIR(PAIR_TREE));
            }
        }
    }

    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + player_col, '|');
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + player_col + 1, '|');
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);

    if (yeti_armed) {
        int yrow = yeti_row_fp / FP_SCALE;
        if (yrow >= 0 && yrow < LINES) {
            attron(COLOR_PAIR(PAIR_YETI) | A_BOLD);
            mvaddch(yrow, PLAYFIELD_X_OFF + yeti_col, 'Y');
            attroff(COLOR_PAIR(PAIR_YETI) | A_BOLD);
        }
    }

    attron(COLOR_PAIR(PAIR_HUD) | A_DIM);
    mvprintw(0, COLS - 13, "DIST %5dm", distance);
    attroff(COLOR_PAIR(PAIR_HUD) | A_DIM);

    refresh();
}

LOCAL int death_screen(void) {
    int cx = COLS / 2;
    int cy = LINES / 2;
    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvprintw(cy - 1, cx - 6, "DIST %5dm", distance);
    mvprintw(cy + 1, cx - 16, "PRESS R TO RESTART  -  Q TO QUIT");
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    refresh();

    timeout(200);
    for (;;) {
        int ch = getch();
        if (sig_quit) return 0;
        if (ch == ERR) continue;
        if (ch == 'r' || ch == 'R') return 1;
        if (ch == 'q' || ch == 'Q' || ch == 27) return 0;
    }
}

LOCAL void play_one_run(void) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    chunk_unpack(BUF_H - CHUNK_ROWS * 2, chunk_pick(0));
    chunk_unpack(BUF_H - CHUNK_ROWS,     chunk_pick(0));

    while (alive && !sig_quit) {
        next.tv_nsec += FRAME_MS * 1000000L;
        if (next.tv_nsec >= 1000000000L) { next.tv_sec += 1; next.tv_nsec -= 1000000000L; }
        int input = 0;
        for (;;) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            long sec = next.tv_sec - now.tv_sec;
            long nsec = next.tv_nsec - now.tv_nsec;
            int ms_left = (int)(sec * 1000 + nsec / 1000000);
            timeout(ms_left > 0 ? ms_left : 0);
            int ch = getch();
            if (ch == ERR) break;
            input = ch;
            if (ch == 'q' || ch == 'Q' || ch == 27) { sig_quit = 1; break; }
        }
        step(input);
        draw();
    }
}
```

And the new `main`:

```c
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    if (!term_init()) return 1;

    game_init((unsigned int)(time(NULL) ^ getpid()));

    while (!sig_quit) {
        play_one_run();
        if (sig_quit || alive) break;
        if (!death_screen()) break;
        game_init((unsigned int)(time(NULL) ^ getpid()));
    }

    term_cleanup();
    return 0;
}
#endif
```

- [ ] **Step 3: Rewrite the tests in `tests/test_yeti.c` for globals**

Every test currently constructs a `struct game g` and passes `&g`. After this refactor those tests reference state that's globally shared. Tests need to call `game_init` to reset (or reset specific fields directly).

Replace the *entire* `tests/test_yeti.c` file with:

```c
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

static void test_xorshift32_deterministic(void) {
    unsigned int s1 = 1, s2 = 1;
    for (int i = 0; i < 100; i++) {
        ASSERT(xorshift32(&s1) == xorshift32(&s2));
    }
}

static void test_xorshift32_different_seeds(void) {
    unsigned int s1 = 1, s2 = 2;
    ASSERT(xorshift32(&s1) != xorshift32(&s2));
}

static void test_world_get_set_roundtrip(void) {
    game_init(1);
    world_set(3, 17, 'T');
    ASSERT(world_get(3, 17) == 'T');
    ASSERT(world_get(3, 18) == 0);
    ASSERT(world_get(4, 17) == 0);
}

static void test_world_get_set_wraps_ring(void) {
    game_init(1);
    world_top = BUF_H - 2;
    world_set(5, 20, 'T');
    /* (BUF_H - 2 + 5) mod BUF_H = 3 */
    ASSERT(world[3][20] == 'T');
    ASSERT(world_get(5, 20) == 'T');
}

static void chunk_render_grid(int chunk_idx, int grid[CHUNK_ROWS][PLAYFIELD_W]) {
    game_init(1);
    chunk_unpack(0, chunk_idx);
    for (int r = 0; r < CHUNK_ROWS; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            grid[r][c] = (world_get(r, c) == 'T') ? 1 : 0;
        }
    }
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
    int grid[CHUNK_ROWS][PLAYFIELD_W];
    for (int i = 0; i < CHUNK_COUNT; i++) {
        chunk_render_grid(i, grid);
        for (int c = 0; c < PLAYFIELD_W; c++) {
            ASSERT(grid[CHUNK_ROWS - 1][c] == 0);
        }
    }
}

static void test_chunk_no_three_consecutive_trees(void) {
    int grid[CHUNK_ROWS][PLAYFIELD_W];
    for (int chunk = 0; chunk < CHUNK_COUNT; chunk++) {
        chunk_render_grid(chunk, grid);
        for (int row = 0; row < CHUNK_ROWS; row++) {
            int run = 0;
            for (int col = 0; col < PLAYFIELD_W; col++) {
                if (grid[row][col]) {
                    run++;
                    ASSERT(run < 3);
                } else {
                    run = 0;
                }
            }
        }
    }
}

static void test_chunk_pick_returns_tier0_when_distance_zero(void) {
    game_init(1);
    for (int i = 0; i < 50; i++) {
        ASSERT(chunk_tier[chunk_pick(0)] == 0);
    }
}

static void test_chunk_pick_returns_tier_1_or_2_when_distance_high(void) {
    game_init(1);
    int seen0 = 0, seen1 = 0, seen2 = 0;
    for (int i = 0; i < 200; i++) {
        int t = chunk_tier[chunk_pick(300)];
        if (t == 0) seen0 = 1;
        if (t == 1) seen1 = 1;
        if (t == 2) seen2 = 1;
    }
    ASSERT(seen0 == 0);
    ASSERT(seen1 == 1);
    ASSERT(seen2 == 1);
}

static void test_chunk_unpack_to_world(void) {
    game_init(1);
    chunk_unpack(0, 0);
    ASSERT(world_get(0, 27) == 'T');
    ASSERT(world_get(0, 26) == 0);
    ASSERT(world_get(1, 12) == 'T');
    ASSERT(world_get(1, 45) == 'T');
    ASSERT(world_get(2, 40) == 'T');
    for (int c = 0; c < PLAYFIELD_W; c++) {
        ASSERT(world_get(3, c) == 0);
    }
}

static void test_world_scroll_advances_top(void) {
    game_init(1);
    int before = world_top;
    world_scroll();
    ASSERT(world_top == (before + 1) % BUF_H);
}

static void test_game_init_with_seed_is_deterministic(void) {
    game_init(42);
    unsigned int a_rng = rng;
    int a_reveal = yeti_reveal_tick;
    int a_col = player_col;
    game_init(42);
    ASSERT(rng == a_rng);
    ASSERT(yeti_reveal_tick == a_reveal);
    ASSERT(player_col == a_col);
    ASSERT(alive == 1);
}

static void test_game_init_reveal_tick_in_range(void) {
    int min_ticks = REVEAL_MIN_S * (1000 / FRAME_MS);
    int max_ticks = REVEAL_MAX_S * (1000 / FRAME_MS);
    for (unsigned int s = 1; s < 50; s++) {
        game_init(s);
        ASSERT(yeti_reveal_tick >= min_ticks);
        ASSERT(yeti_reveal_tick <  max_ticks);
    }
}

static void test_game_init_starting_state(void) {
    game_init(7);
    ASSERT(tick == 0);
    ASSERT(distance == 0);
    ASSERT(alive == 1);
    ASSERT(yeti_armed == 0);
    ASSERT(player_col == PLAYFIELD_W / 2 - 1);
    ASSERT(scroll_period == SCROLL_PERIOD_START);
}

static void test_player_move_left(void) {
    game_init(1);
    int before = player_col;
    player_step(KEY_LEFT);
    ASSERT(player_col == before - 1);
}

static void test_player_move_right(void) {
    game_init(1);
    int before = player_col;
    player_step(KEY_RIGHT);
    ASSERT(player_col == before + 1);
}

static void test_player_clamps_at_left(void) {
    game_init(1);
    player_col = 0;
    player_step(KEY_LEFT);
    ASSERT(player_col == 0);
}

static void test_player_clamps_at_right(void) {
    game_init(1);
    player_col = PLAYFIELD_W - 2;
    player_step(KEY_RIGHT);
    ASSERT(player_col == PLAYFIELD_W - 2);
}

static void test_player_collision_left_cell(void) {
    game_init(1);
    world_set(PLAYER_ROW, player_col, 'T');
    player_check_collision();
    ASSERT(alive == 0);
}

static void test_player_collision_right_cell(void) {
    game_init(1);
    world_set(PLAYER_ROW, player_col + 1, 'T');
    player_check_collision();
    ASSERT(alive == 0);
}

static void test_player_no_collision_when_clear(void) {
    game_init(1);
    player_check_collision();
    ASSERT(alive == 1);
}

static void test_yeti_pre_reveal_does_nothing(void) {
    game_init(1);
    tick = 100;
    int before_row = yeti_row_fp;
    yeti_step(0);
    ASSERT(yeti_armed == 0);
    ASSERT(yeti_row_fp == before_row);
}

static void test_yeti_arms_at_reveal_tick(void) {
    game_init(1);
    tick = yeti_reveal_tick;
    yeti_step(0);
    ASSERT(yeti_armed == 1);
    ASSERT(yeti_col == PLAYFIELD_W / 2);
    ASSERT(scroll_period == SCROLL_PERIOD_START - 1);
}

static void test_yeti_closes_baseline(void) {
    game_init(1);
    yeti_armed = 1;
    yeti_row_fp = 0;
    yeti_step(0);
    ASSERT(yeti_row_fp == BASE_CLOSING_FP);
}

static void test_yeti_press_cost_adds_to_closing(void) {
    game_init(1);
    yeti_armed = 1;
    yeti_row_fp = 0;
    yeti_step(KEY_LEFT);
    ASSERT(yeti_row_fp == BASE_CLOSING_FP + PRESS_COST_FP);
}

static void test_yeti_horizontal_drift_toward_player(void) {
    game_init(1);
    yeti_armed = 1;
    yeti_col = 10;
    player_col = 30;
    yeti_step(0);
    ASSERT(yeti_col == 11);
    yeti_col = 30;
    player_col = 5;
    yeti_step(0);
    ASSERT(yeti_col == 29);
}

static void test_step_increments_tick(void) {
    game_init(1);
    step(0);
    ASSERT(tick == 1);
    step(0);
    ASSERT(tick == 2);
}

static void test_step_does_nothing_when_dead(void) {
    game_init(1);
    alive = 0;
    int before = tick;
    step(KEY_LEFT);
    ASSERT(tick == before);
}

int main(void) {
    test_xorshift32_deterministic();
    test_xorshift32_different_seeds();
    test_world_get_set_roundtrip();
    test_world_get_set_wraps_ring();
    test_chunk_count_matches_tiers();
    test_chunk_last_row_always_empty();
    test_chunk_no_three_consecutive_trees();
    test_chunk_pick_returns_tier0_when_distance_zero();
    test_chunk_pick_returns_tier_1_or_2_when_distance_high();
    test_chunk_unpack_to_world();
    test_world_scroll_advances_top();
    test_game_init_with_seed_is_deterministic();
    test_game_init_reveal_tick_in_range();
    test_game_init_starting_state();
    test_player_move_left();
    test_player_move_right();
    test_player_clamps_at_left();
    test_player_clamps_at_right();
    test_player_collision_left_cell();
    test_player_collision_right_cell();
    test_player_no_collision_when_clear();
    test_yeti_pre_reveal_does_nothing();
    test_yeti_arms_at_reveal_tick();
    test_yeti_closes_baseline();
    test_yeti_press_cost_adds_to_closing();
    test_yeti_horizontal_drift_toward_player();
    test_step_increments_tick();
    test_step_does_nothing_when_dead();
    fprintf(stderr, "\n%d/%d tests passed\n",
            test_count - fail_count, test_count);
    return fail_count ? 1 : 0;
}
```

(Tests that exercised `step` with simulated multi-tick loops are dropped — they're brittle against the global state and the surviving tests cover the same logic via direct field manipulation.)

- [ ] **Step 4: Build and test**

```bash
make clean && make && make test && make size
```

Expected: builds; ~225 tests pass (down from ~3,500 because most of the noise came from the dropped 100/200/1000-iteration loops); `make size` reports ~10,740 ± 200.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "$(cat <<'EOF'
refactor(game): promote struct game to file-scope globals

Phase 6 of the byte-budget reduction. struct game disappears.
Every field becomes a file-scope LOCAL variable; every function
loses its struct game * parameter and every body loses its g->.

Tests rewritten against the new shape: each test calls game_init
to reset state, then manipulates globals directly.

No player-visible behavior change. Saves ~600 bytes by eliminating
roughly 80 g-> prefixes and the struct passing in signatures.
EOF
)"
```

---

### Task 8: Phase 7 — Light code-golf hot paths (CONDITIONAL)

**Run only if Task 6 (or post-Task-7 measurement) says so.**

**Files:**
- Modify: `src/yeti.c`

**Goal:** Tighten `draw`, `step`, `play_one_run` without sacrificing readability. Target saving: ~500 bytes. Landing: ~10,240.

Each step here is a targeted local change; the file structure stays as-is.

- [ ] **Step 1: Tighten `draw`**

Replace `draw` with:

```c
LOCAL void draw(void) {
    erase();
    for (int r = 0, n = LINES < BUF_H ? LINES : BUF_H; r < n; r++)
        for (int c = 0; c < PLAYFIELD_W; c++)
            if (world_get(r, c) == 'T') {
                attron(COLOR_PAIR(PAIR_TREE));
                mvaddch(r, PLAYFIELD_X_OFF + c, 'T');
                attroff(COLOR_PAIR(PAIR_TREE));
            }

    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + player_col,     '|');
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + player_col + 1, '|');
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);

    if (yeti_armed) {
        int y = yeti_row_fp / FP_SCALE;
        if (y >= 0 && y < LINES) {
            attron(COLOR_PAIR(PAIR_YETI) | A_BOLD);
            mvaddch(y, PLAYFIELD_X_OFF + yeti_col, 'Y');
            attroff(COLOR_PAIR(PAIR_YETI) | A_BOLD);
        }
    }

    attron(COLOR_PAIR(PAIR_HUD) | A_DIM);
    mvprintw(0, COLS - 13, "DIST %5dm", distance);
    attroff(COLOR_PAIR(PAIR_HUD) | A_DIM);
    refresh();
}
```

- [ ] **Step 2: Tighten `step`**

Replace `step` with:

```c
LOCAL void step(int input) {
    if (!alive) return;
    tick++;

    int span = SCROLL_PERIOD_START - SCROLL_PERIOD_END;
    int reduced = (tick * span) / (60 * (1000 / FRAME_MS));
    if (reduced > span) reduced = span;
    int target = SCROLL_PERIOD_START - reduced;
    if (target < scroll_period) scroll_period = target;

    player_step(input);
    yeti_step(input);

    if (++ticks_since_scroll >= scroll_period) {
        ticks_since_scroll = 0;
        world_scroll();
        if ((++distance % CHUNK_ROWS) == 0)
            chunk_unpack(BUF_H - CHUNK_ROWS, chunk_pick(distance));
    }
    player_check_collision();
}
```

- [ ] **Step 3: Tighten `play_one_run`**

Replace `play_one_run` with:

```c
LOCAL void play_one_run(void) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    chunk_unpack(BUF_H - CHUNK_ROWS * 2, chunk_pick(0));
    chunk_unpack(BUF_H - CHUNK_ROWS,     chunk_pick(0));

    while (alive && !sig_quit) {
        next.tv_nsec += FRAME_MS * 1000000L;
        if (next.tv_nsec >= 1000000000L) { next.tv_sec++; next.tv_nsec -= 1000000000L; }
        int input = 0;
        for (;;) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            int ms = (int)((next.tv_sec - now.tv_sec) * 1000
                         + (next.tv_nsec - now.tv_nsec) / 1000000);
            timeout(ms > 0 ? ms : 0);
            int ch = getch();
            if (ch == ERR) break;
            input = ch;
            if (ch == 'q' || ch == 'Q' || ch == 27) { sig_quit = 1; break; }
        }
        step(input);
        draw();
    }
}
```

- [ ] **Step 4: Tighten `chunk_pick`**

Replace `chunk_pick` with:

```c
LOCAL int chunk_pick(int dist) {
    int lo = dist < 100 ? 0 : (dist < 200 ? 0 : 1);
    int hi = dist < 100 ? 0 : (dist < 200 ? 1 : 2);
    for (;;) {
        int i = (int)(xorshift32(&rng) % CHUNK_COUNT);
        if (chunk_tier[i] >= lo && chunk_tier[i] <= hi) return i;
    }
}
```

- [ ] **Step 5: Drop the `cy` intermediate in `death_screen`**

Replace `death_screen` with:

```c
LOCAL int death_screen(void) {
    int cx = COLS / 2, cy = LINES / 2;
    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvprintw(cy - 1, cx - 6,  "DIST %5dm", distance);
    mvprintw(cy + 1, cx - 16, "PRESS R TO RESTART  -  Q TO QUIT");
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    refresh();

    timeout(200);
    for (;;) {
        int ch = getch();
        if (sig_quit) return 0;
        if (ch == ERR) continue;
        if (ch == 'r' || ch == 'R') return 1;
        if (ch == 'q' || ch == 'Q' || ch == 27) return 0;
    }
}
```

- [ ] **Step 6: Build and test**

```bash
make clean && make && make test && make size
```

Expected: builds; tests still pass (count unchanged from phase 6); `make size` reports ~10,240 ± 200.

- [ ] **Step 7: Commit**

```bash
git add src/yeti.c
git commit -m "$(cat <<'EOF'
refactor(game): code-golf the hot paths

Phase 7 of the byte-budget reduction. Tightens draw, step,
play_one_run, chunk_pick, and death_screen with single-statement
loop bodies, combined declarations, ?: ternaries, and shorter
local-only names (`y` for the computed yeti row, `lo`/`hi` for
tier bounds). No function rename, no signature change, no
player-visible behavior change.
EOF
)"
```

---

### Task 9: Phase 8 — Drop test scaffolding (always run)

**Files:**
- Modify: `src/yeti.c`
- Delete: `tests/test_yeti.c`
- Modify: `Makefile`

**Goal:** Final ~90-byte trim. Test build goes away.

- [ ] **Step 1: Remove the `LOCAL` macro and `#ifdef GAME_TEST` guards**

In `src/yeti.c`, delete:

```c
/* --- test visibility --- */
#ifdef GAME_TEST
#define LOCAL
#else
#define LOCAL static
#endif
```

Then sweep the file: every occurrence of `LOCAL` becomes `static`. Also delete the `#ifndef GAME_TEST` / `#endif` guards around `main`.

- [ ] **Step 2: Delete the test file**

```bash
rm tests/test_yeti.c
rmdir tests  # if empty
```

- [ ] **Step 3: Remove test target from `Makefile`**

In `Makefile`, delete:

```make
test: tests/test_yeti
	@./tests/test_yeti

tests/test_yeti: tests/test_yeti.c src/yeti.c
	$(CC) $(CFLAGS) -Wno-missing-prototypes -DGAME_TEST -o $@ tests/test_yeti.c $(LDFLAGS) $(LIBS)
```

Update the `.PHONY` line. Change:

```make
.PHONY: all clean install uninstall size test
```

to:

```make
.PHONY: all clean install uninstall size
```

Also remove the now-stale `tests/test_yeti` from the `clean` target. Change:

```make
clean:
	rm -f $(BIN) tests/test_yeti
```

to:

```make
clean:
	rm -f $(BIN)
```

- [ ] **Step 4: Build**

```bash
make clean && make && make size
```

Expected: builds; `make size` reports ≤ 10,240.

- [ ] **Step 5: Manual playthrough**

```bash
./yeti
```

Play a few runs. Verify:
- Skier (`||`) at row 7, world scrolling, trees (`T` green) appearing.
- Left/right moves the skier; press cost is visible as the yeti closes faster when you move.
- Tree contact ends the run; death screen overlays distance + "PRESS R TO RESTART  -  Q TO QUIT".
- `r` starts a new run; `q` / ESC quits cleanly with the terminal restored.
- Yeti (`Y` red bold) appears between 30 s and 60 s and closes from the top.
- Ctrl-C during death screen exits cleanly within ~200 ms.

- [ ] **Step 6: Commit**

```bash
git add src/yeti.c Makefile
git rm tests/test_yeti.c
git commit -m "$(cat <<'EOF'
refactor(test): drop test scaffolding and tests/test_yeti.c

Phase 8 of the byte-budget reduction. The LOCAL macro and GAME_TEST
guards exist purely to let tests/test_yeti.c re-include src/yeti.c
with external linkage. With the budget hit, the scaffolding is no
longer worth its bytes (~90B). The test file, Makefile test target,
and tests/ directory go too.

Future test work will need PTY-based integration tests (separate
session). The design spec lists this in the "out of scope" block.
EOF
)"
```

---

## Self-review

**Spec coverage check:** Eight phases, eight tasks (plus the conditional checkpoint as Task 6). Each phase in the spec maps to a single task. Phases 6 and 7 are conditional on the checkpoint, matching the spec's "stop the moment we hit budget" rule.

**Placeholder scan:** No TBDs. Every step has concrete code or a literal command. The conditional branches in Task 6 give explicit decision rules.

**Type consistency:** After Phase 6 (Task 7), the entire codebase no longer uses `struct game`. Tasks 8 and 9 are written against the post-phase-6 shape. Earlier tasks (1-5) keep `struct game` intact; only the conditional Task 7 introduces the change. Function signatures in Task 8 match Task 7's post-refactor shape exactly.

**Conditional coverage:** Task 6 (the checkpoint) gives clear thresholds for which conditional tasks to run. Task 9 (phase 8) is always last regardless of whether 7 or 8 ran.
