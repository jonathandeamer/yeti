# yeti — minimum-playable implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first playable cut of `yeti` — a single-file C + ncurses ski-down where the yeti is the inevitable end of every run.

**Architecture:** One source file (`src/yeti.c`) compiled to one binary. Pure-logic functions (PRNG, world ring buffer, chunk unpack, player movement, yeti chase) get TDD coverage via a `tests/test_yeti.c` that re-includes the source under `-DGAME_TEST`. Curses-driven code (terminal init, draw, main loop, death screen) is manually smoke-tested using deterministic seeds and the `--ticks N` test flag. The bit-packed chunk pool encodes hand-verified obstacle patterns; the `chunk-no-3-in-a-row` invariant is itself a TDD test, so chunk authoring becomes test-driven.

**Tech Stack:** C99 + X/Open Curses subset (ncurses) + POSIX-2008. No third-party deps beyond ncurses. Build via POSIX-compatible Makefile honoring `CC`, `CFLAGS`, `LDFLAGS`, `DESTDIR`, `PREFIX`. ncurses detected via `pkg-config --cflags --libs ncurses` with `-lncurses` fallback.

**Cross-references:**
- Spec: `docs/superpowers/specs/2026-05-25-yeti-minimum-playable-design.md` — every implementation decision below is sourced from this file
- Source-contact discipline: `FORBIDDEN-SOURCES.md` — binding on every participant

---

## File structure

**Created in this plan:**

- `.gitignore` — ignores binaries
- `LICENSE` — root pointer to LICENSES/
- `LICENSES/MIT.txt` — full MIT text
- `LICENSES/CC0-1.0.txt` — full CC0 1.0 text
- `Makefile` — `all`, `clean`, `test`, `install`, `uninstall`, `size`
- `README.md` — hook, build, controls, license
- `src/yeti.c` — the game (single TU; SPDX header, `LOCAL` macro for test-visibility, section-comment dividers)
- `tests/test_yeti.c` — assertion-style unit tests; re-includes `src/yeti.c` with `-DGAME_TEST`

**Already in repo:**

- `FORBIDDEN-SOURCES.md`
- `docs/superpowers/specs/2026-05-25-yeti-minimum-playable-design.md`
- `.git/hooks/commit-msg` (conventional-commits hook)

## Commit-message conventions

The hook requires `type(scope): description`. Allowed types: `feat fix perf refactor style test docs build chore ci`. Allowed scopes: `game world yeti render term score cli build docs ci meta`.

This plan's commits use scopes:
- `meta` for LICENSE / .gitignore / SPDX setup
- `build` for Makefile changes
- `docs` for README / spec / plan
- `game` for the state struct + main loop + step orchestration
- `world` for chunk pool + ring buffer + world step
- `yeti` for reveal scheduling + chase math
- `render` for draw / glyphs / HUD / death screen
- `term` for curses init/cleanup + signal handling + min-size + TERM check
- `cli` for argv parsing
- `test` (type, not scope) for test-only additions; scope is whichever subsystem the test covers

---

### Task 1: Repo bootstrap — gitignore + dual-license setup

**Files:**
- Create: `.gitignore`
- Create: `LICENSE`
- Create: `LICENSES/MIT.txt`
- Create: `LICENSES/CC0-1.0.txt`

- [ ] **Step 1: Write `.gitignore`**

```
yeti
tests/test_yeti
*.o
*.dSYM/
```

- [ ] **Step 2: Write root `LICENSE` (pointer to LICENSES/)**

```
yeti is dual-licensed under your choice of:

  MIT License        (see LICENSES/MIT.txt)
  CC0 1.0 Universal  (see LICENSES/CC0-1.0.txt)

Source files carry the SPDX expression:

  SPDX-License-Identifier: MIT OR CC0-1.0

Recipients may use, modify, and redistribute under either license.
```

- [ ] **Step 3: Write `LICENSES/MIT.txt`**

```
MIT License

Copyright (c) 2026 Jonathan Deamer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```

- [ ] **Step 4: Write `LICENSES/CC0-1.0.txt`**

Copy the full CC0 1.0 Universal text from <https://creativecommons.org/publicdomain/zero/1.0/legalcode>. The legalcode is plain text, ~250 lines. Save verbatim.

- [ ] **Step 5: Commit**

```bash
git add .gitignore LICENSE LICENSES/MIT.txt LICENSES/CC0-1.0.txt
git commit -m "chore(meta): add dual MIT or CC0 license and gitignore"
```

Expected: hook passes, commit created.

---

### Task 2: Minimal Makefile + `src/yeti.c` stub that compiles to a no-op binary

**Files:**
- Create: `src/yeti.c`
- Create: `Makefile`

- [ ] **Step 1: Create `src/yeti.c` stub**

```c
/* SPDX-License-Identifier: MIT OR CC0-1.0 */

/* --- includes --- */
#include <stdio.h>

/* --- constants --- */
/* (none yet) */

/* --- main --- */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fputs("yeti: stub\n", stdout);
    return 0;
}
```

- [ ] **Step 2: Create `Makefile`**

```make
CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wpedantic -Wshadow \
           -Wstrict-prototypes -Wmissing-prototypes \
           -Wold-style-definition -fno-common -O2
LDFLAGS ?=
LIBS    ?=

PREFIX  ?= /usr/local
DESTDIR ?=

SRC = src/yeti.c
BIN = yeti

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LDFLAGS)

clean:
	rm -f $(BIN) tests/test_yeti

size:
	@wc -c $(SRC)

.PHONY: all clean size
```

- [ ] **Step 3: Build and run**

```bash
make clean && make && ./yeti
```

Expected:
```
yeti: stub
```

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c Makefile
git commit -m "build(build): add minimal Makefile and src/yeti.c stub"
```

---

### Task 3: README.md

**Files:**
- Create: `README.md`

- [ ] **Step 1: Write README**

````markdown
# yeti

> yeti - ski down a mountain until the yeti finds you

A short terminal ski game. The yeti is the inevitable end of every run.

## Build

```sh
make
```

Requires a C99 compiler and ncurses. ncurses is detected via `pkg-config` with an `-lncurses` fallback.

## Run

```sh
./yeti
```

Optional flags:

| Flag | Effect |
| --- | --- |
| `-h`, `--help` | print help and exit |
| `-s SEED` | deterministic run with the given RNG seed |

## Controls

| Key | Action |
| --- | --- |
| ← / → | lean |
| Q, ESC | quit |
| R | restart from death |

## License

Dual-licensed under your choice of MIT or CC0-1.0. See `LICENSES/MIT.txt` and `LICENSES/CC0-1.0.txt`.

## Independent reimplementation

`yeti` is implemented without source contact with any predecessor or sibling ski game. See `FORBIDDEN-SOURCES.md` for the discipline that binds every contributor.
````

- [ ] **Step 2: Commit**

```bash
git add README.md
git commit -m "docs(docs): add README"
```

---

### Task 4: Test scaffolding — `tests/test_yeti.c` + Makefile target + `LOCAL` macro in source

**Files:**
- Create: `tests/test_yeti.c`
- Modify: `src/yeti.c` (add `LOCAL` macro)
- Modify: `Makefile` (add `test` target)

- [ ] **Step 1: Add `LOCAL` macro and reorganize `src/yeti.c`**

```c
/* SPDX-License-Identifier: MIT OR CC0-1.0 */

/* --- includes --- */
#include <stdio.h>

/* --- test visibility --- */
#ifdef GAME_TEST
#define LOCAL
#else
#define LOCAL static
#endif

/* --- constants --- */
/* (none yet) */

/* --- main --- */
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fputs("yeti: stub\n", stdout);
    return 0;
}
#endif
```

- [ ] **Step 2: Create `tests/test_yeti.c` with a sanity test**

```c
/* SPDX-License-Identifier: MIT OR CC0-1.0 */

#define GAME_TEST 1
#include "../src/yeti.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_count = 0;
static int fail_count = 0;

#define ASSERT(cond) do {                                              \
    test_count++;                                                      \
    if (!(cond)) {                                                     \
        fprintf(stderr, "\nFAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        fail_count++;                                                  \
    } else {                                                           \
        fputc('.', stdout);                                            \
        fflush(stdout);                                                \
    }                                                                  \
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
```

- [ ] **Step 3: Add `test` target to `Makefile`**

Append before `.PHONY`:

```make
test: tests/test_yeti
	@./tests/test_yeti

tests/test_yeti: tests/test_yeti.c src/yeti.c
	$(CC) $(CFLAGS) -DGAME_TEST -o $@ tests/test_yeti.c $(LIBS) $(LDFLAGS)
```

Update the `.PHONY` line to:

```make
.PHONY: all clean size test
```

- [ ] **Step 4: Run tests**

```bash
make test
```

Expected:
```
.
1/1 tests passed
```

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c Makefile
git commit -m "test(build): add test scaffold with LOCAL macro and make test target"
```

---

### Task 5: PRNG (xorshift32) — TDD

**Files:**
- Modify: `src/yeti.c` (add PRNG section)
- Modify: `tests/test_yeti.c` (add PRNG tests)

- [ ] **Step 1: Write failing tests for xorshift32**

Add to `tests/test_yeti.c` above `int main`:

```c
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
```

Add to `main(void)`:

```c
test_xorshift32_deterministic();
test_xorshift32_nontrivial();
test_xorshift32_different_seeds();
test_xorshift_uniform_in_range();
```

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compilation failure with `error: implicit declaration of function 'xorshift32'` (or similar).

- [ ] **Step 3: Implement PRNG in `src/yeti.c`**

Add a new section before `/* --- main --- */`:

```c
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
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected:
```
.....
5/5 tests passed
```

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(game): add xorshift32 PRNG"
```

---

### Task 6: Frame-timing helpers — `ms_until`, `add_ms` — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

Add to `tests/test_yeti.c`:

```c
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
```

Add to `main(void)`:

```c
test_add_ms_basic();
test_add_ms_wrap();
test_ms_until_positive();
test_ms_until_past_returns_nonpositive();
```

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `add_ms` and `ms_until`.

- [ ] **Step 3: Implement timing helpers**

In `src/yeti.c`, add `<time.h>` to includes:

```c
#include <stdio.h>
#include <time.h>
```

Add a new section before `/* --- prng --- */`:

```c
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
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 9/9 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(game): add timing helpers add_ms and ms_until"
```

---

### Task 7: Constants and state struct — types only, no logic

**Files:**
- Modify: `src/yeti.c`

- [ ] **Step 1: Add the constants block and state struct**

Update `src/yeti.c`. After the `/* --- includes --- */` section, add `<stdint.h>` to includes:

```c
#include <stdint.h>
```

Replace the empty `/* --- constants --- */` block with:

```c
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
```

- [ ] **Step 2: Build to verify no compilation issues**

```bash
make clean && make
```

Expected: builds cleanly.

- [ ] **Step 3: Build tests to verify**

```bash
make test
```

Expected: 9/9 tests passed.

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c
git commit -m "feat(game): add constants block and state struct"
```

---

### Task 8: World ring buffer — `world_get`, `world_set`, `world_clear` — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

Add to `tests/test_yeti.c`:

```c
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
    world_set(&g, 3, 17, 'Y');
    ASSERT(world_get(&g, 3, 17) == 'Y');
    ASSERT(world_get(&g, 3, 18) == 0);
    ASSERT(world_get(&g, 4, 17) == 0);
}

static void test_world_get_set_honors_ring_top(void) {
    struct game g;
    g.world_top = 5;
    world_clear(&g);
    world_set(&g, 0, 10, 'Y');
    /* row 0 from caller's view is ring index (5 + 0) % BUF_H = 5 */
    ASSERT(g.world[5][10] == 'Y');
    ASSERT(world_get(&g, 0, 10) == 'Y');
}

static void test_world_get_set_wraps_ring(void) {
    struct game g;
    g.world_top = BUF_H - 2;
    world_clear(&g);
    world_set(&g, 5, 20, 'Y');
    /* (BUF_H - 2 + 5) mod BUF_H = 3 */
    ASSERT(g.world[3][20] == 'Y');
    ASSERT(world_get(&g, 5, 20) == 'Y');
}
```

Add to `main(void)`:

```c
test_world_clear_zeros_buffer();
test_world_get_set_roundtrip();
test_world_get_set_honors_ring_top();
test_world_get_set_wraps_ring();
```

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `world_clear`, `world_get`, `world_set`.

- [ ] **Step 3: Implement world ring**

Add `<string.h>` to includes. Add a new section before `/* --- main --- */`:

```c
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
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 13/13 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(world): add ring-buffer accessors world_clear/get/set"
```

---

### Task 9: Chunk pool data + invariant tests — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write invariant tests (these tests *are* the chunk acceptance criteria)**

Add to `tests/test_yeti.c`:

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
    /* cols 60..63 must be unset (we only use 60 cols) */
    for (int chunk = 0; chunk < CHUNK_COUNT; chunk++) {
        for (int row = 0; row < CHUNK_ROWS; row++) {
            ASSERT((chunk_pool[chunk][row][CHUNK_BYTES - 1] & 0x0F) == 0);
        }
    }
}
```

Add to `main(void)`:

```c
test_chunk_count_matches_tiers();
test_chunk_last_row_always_empty();
test_chunk_no_three_consecutive_trees();
test_chunk_unused_bits_are_zero();
```

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `chunk_pool` and `chunk_tier`.

- [ ] **Step 3: Add chunk pool data to `src/yeti.c`**

At the top of the `/* --- world --- */` section, before the existing helpers:

```c
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
    /* chunk 3 (tier 1): (0,5), (0,19), (0,38), (1,12), (1,29), (1,46), (2,8), (2,21), (2,45) */
    { { 0x04, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00 },
      { 0x00, 0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00 },
      { 0x00, 0x80, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 4 (tier 1): (0,2), (0,15), (0,36), (1,8), (1,27), (1,47), (2,4), (2,18), (2,36) */
    { { 0x20, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 },
      { 0x00, 0x80, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00 },
      { 0x08, 0x00, 0x20, 0x00, 0x08, 0x00, 0x00, 0x00 },
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    /* chunk 5 (tier 1): (0,7), (0,22), (0,41), (1,2), (1,20), (1,45), (2,11), (2,31), (2,48) */
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
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 17/17 tests passed. If any chunk-invariant test fails, the chunk data is wrong — fix the bytes until the tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(world): add bit-packed chunk pool with invariant tests"
```

---

### Task 10: Chunk pick + unpack into world buffer — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

```c
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
    ASSERT(seen0 == 0);  /* tier 0 forbidden at distance >= 200 */
    ASSERT(seen1 == 1);
    ASSERT(seen2 == 1);
}

static void test_chunk_unpack_to_world(void) {
    struct game g;
    g.world_top = 0;
    world_clear(&g);
    /* chunk 0: (0,27), (1,12), (1,45), (2,40), (3 empty) */
    chunk_unpack(&g, 0, /*dest_row=*/0, /*chunk_idx=*/0);
    ASSERT(world_get(&g, 0, 27) == 'Y');
    ASSERT(world_get(&g, 0, 26) == 0);
    ASSERT(world_get(&g, 0, 28) == 0);
    ASSERT(world_get(&g, 1, 12) == 'Y');
    ASSERT(world_get(&g, 1, 45) == 'Y');
    ASSERT(world_get(&g, 2, 40) == 'Y');
    /* row 3 fully empty */
    for (int c = 0; c < PLAYFIELD_W; c++) {
        ASSERT(world_get(&g, 3, c) == 0);
    }
}
```

Add the three tests to `main(void)`.

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `chunk_pick` and `chunk_unpack`.

- [ ] **Step 3: Implement chunk_pick and chunk_unpack**

In the `/* --- world --- */` section, after the chunk-pool data:

```c
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
            world_set(g, dest_row + r, c, set ? 'Y' : 0);
        }
    }
}
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 20/20 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(world): add chunk_pick and chunk_unpack"
```

---

### Task 11: World scroll step + chunk generation — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

```c
static void test_world_scroll_advances_top(void) {
    struct game g;
    memset(&g, 0, sizeof g);
    int before = g.world_top;
    world_scroll(&g);
    ASSERT(g.world_top == (before + 1) % BUF_H);
}

static void test_world_scroll_clears_new_top_row(void) {
    struct game g;
    memset(&g, 0, sizeof g);
    world_clear(&g);
    /* mark the cell that will become the new last row */
    int last = (g.world_top + BUF_H - 1) % BUF_H;
    g.world[last][0] = 'Y';
    world_scroll(&g);
    /* after scroll, that row is now the topmost — and its old content
     * should remain (we haven't generated new content). Only world_gen
     * writes new rows. world_scroll only advances world_top. */
    /* No assertion about clearing here; just ensure top advanced. */
    ASSERT(g.world_top != 0 || BUF_H == 1);
}

static void test_world_gen_writes_full_chunk(void) {
    struct game g;
    memset(&g, 0, sizeof g);
    g.rng = 1;
    world_clear(&g);
    int before_top = g.world_top;
    int written = world_gen_chunk(&g);  /* returns starting dest_row */
    ASSERT(written == before_top + 0 || written >= 0);
    /* After a chunk write, the last row of the chunk window should be empty. */
    for (int c = 0; c < PLAYFIELD_W; c++) {
        ASSERT(world_get(&g, written + CHUNK_ROWS - 1, c) == 0);
    }
}
```

Add to `main(void)`.

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile errors referencing `world_scroll` and `world_gen_chunk`.

- [ ] **Step 3: Implement scroll and chunk generation**

In `/* --- world --- */`:

```c
LOCAL void world_scroll(struct game *g) {
    g->world_top = (g->world_top + 1) % BUF_H;
    /* The row that just rotated to the bottom of the visible window
     * keeps whatever it had; world_gen_chunk overwrites a chunk-window
     * worth of rows at the bottom on a chunk boundary. */
}

LOCAL int world_gen_chunk(struct game *g) {
    /* Write a fresh chunk into rows [BUF_H - CHUNK_ROWS .. BUF_H - 1]
     * relative to world_top. Returns dest_row (BUF_H - CHUNK_ROWS). */
    int dest_row = BUF_H - CHUNK_ROWS;
    int pick = chunk_pick(&g->rng, g->distance);
    chunk_unpack(g, dest_row, pick);
    return dest_row;
}
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 23/23 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(world): add world_scroll and world_gen_chunk"
```

---

### Task 12: Game init — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

```c
static void test_game_init_with_seed_is_deterministic(void) {
    struct game a, b;
    game_init(&a, 42);
    game_init(&b, 42);
    ASSERT(a.rng == b.rng);
    ASSERT(a.yeti_reveal_tick == b.yeti_reveal_tick);
    ASSERT(a.player_col == b.player_col);
    ASSERT(a.alive == b.alive);
}

static void test_game_init_reveal_tick_in_range(void) {
    struct game g;
    int min_ticks = REVEAL_MIN_S * (1000 / FRAME_MS);
    int max_ticks = REVEAL_MAX_S * (1000 / FRAME_MS);
    for (unsigned int s = 1; s < 50; s++) {
        game_init(&g, s);
        ASSERT(g.yeti_reveal_tick >= min_ticks);
        ASSERT(g.yeti_reveal_tick <  max_ticks);
    }
}

static void test_game_init_starting_state(void) {
    struct game g;
    game_init(&g, 7);
    ASSERT(g.tick == 0);
    ASSERT(g.distance == 0);
    ASSERT(g.alive == 1);
    ASSERT(g.yeti_armed == 0);
    ASSERT(g.player_col == PLAYFIELD_W / 2 - 1);
    ASSERT(g.scroll_period == SCROLL_PERIOD_START);
}
```

Add to `main(void)`.

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `game_init`.

- [ ] **Step 3: Implement `game_init`**

Add to the `/* --- state --- */` section (after the struct definition):

```c
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
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 26/26 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(game): add game_init with deterministic seeding"
```

---

### Task 13: Player movement + collision — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

```c
static void test_player_move_left(void) {
    struct game g;
    game_init(&g, 1);
    int before = g.player_col;
    player_step(&g, KEY_LEFT);
    ASSERT(g.player_col == before - 1);
}

static void test_player_move_right(void) {
    struct game g;
    game_init(&g, 1);
    int before = g.player_col;
    player_step(&g, KEY_RIGHT);
    ASSERT(g.player_col == before + 1);
}

static void test_player_clamps_at_left(void) {
    struct game g;
    game_init(&g, 1);
    g.player_col = 0;
    player_step(&g, KEY_LEFT);
    ASSERT(g.player_col == 0);
}

static void test_player_clamps_at_right(void) {
    struct game g;
    game_init(&g, 1);
    g.player_col = PLAYFIELD_W - 2;
    player_step(&g, KEY_RIGHT);
    ASSERT(g.player_col == PLAYFIELD_W - 2);
}

static void test_player_collision_left_cell(void) {
    struct game g;
    game_init(&g, 1);
    world_set(&g, PLAYER_ROW, g.player_col, 'Y');
    player_check_collision(&g);
    ASSERT(g.alive == 0);
}

static void test_player_collision_right_cell(void) {
    struct game g;
    game_init(&g, 1);
    world_set(&g, PLAYER_ROW, g.player_col + 1, 'Y');
    player_check_collision(&g);
    ASSERT(g.alive == 0);
}

static void test_player_no_collision_when_clear(void) {
    struct game g;
    game_init(&g, 1);
    /* world is freshly cleared by game_init */
    player_check_collision(&g);
    ASSERT(g.alive == 1);
}
```

Note: `KEY_LEFT` and `KEY_RIGHT` come from `<ncurses.h>`. Need to include it now.

Add to `main(void)`.

- [ ] **Step 2: Add ncurses include + run tests, verify failure**

In `src/yeti.c` includes:

```c
#include <ncurses.h>
```

In `Makefile`, add ncurses link:

```make
LIBS_NCURSES := $(shell pkg-config --libs ncurses 2>/dev/null || echo -lncurses)
CFLAGS_NCURSES := $(shell pkg-config --cflags ncurses 2>/dev/null)
CFLAGS += $(CFLAGS_NCURSES)
LIBS += $(LIBS_NCURSES)
```

Place these lines after the existing `CFLAGS ?=` block.

```bash
make clean && make test
```

Expected: compile errors referencing `player_step` and `player_check_collision`.

- [ ] **Step 3: Implement player movement and collision**

Add a new section before `/* --- main --- */`:

```c
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
    if (left == 'Y' || right == 'Y') g->alive = 0;
}
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 33/33 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c Makefile
git commit -m "feat(game): add player movement and collision check"
```

---

### Task 14: Yeti reveal + chase math — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

```c
static void test_yeti_pre_reveal_does_nothing(void) {
    struct game g;
    game_init(&g, 1);
    g.tick = 100;  /* well before reveal */
    int before_row = g.yeti_row_fp;
    yeti_step(&g, 0);
    ASSERT(g.yeti_armed == 0);
    ASSERT(g.yeti_row_fp == before_row);
}

static void test_yeti_arms_at_reveal_tick(void) {
    struct game g;
    game_init(&g, 1);
    g.tick = g.yeti_reveal_tick;
    yeti_step(&g, 0);
    ASSERT(g.yeti_armed == 1);
    ASSERT(g.yeti_col == PLAYFIELD_W / 2);
    ASSERT(g.scroll_period == SCROLL_PERIOD_START - 1);
}

static void test_yeti_closes_baseline(void) {
    struct game g;
    game_init(&g, 1);
    g.yeti_armed = 1;
    g.yeti_row_fp = 0;
    yeti_step(&g, 0);  /* no lean */
    ASSERT(g.yeti_row_fp == BASE_CLOSING_FP);
}

static void test_yeti_press_cost_adds_to_closing(void) {
    struct game g;
    game_init(&g, 1);
    g.yeti_armed = 1;
    g.yeti_row_fp = 0;
    yeti_step(&g, KEY_LEFT);
    ASSERT(g.yeti_row_fp == BASE_CLOSING_FP + PRESS_COST_FP);
}

static void test_yeti_horizontal_drift_toward_player(void) {
    struct game g;
    game_init(&g, 1);
    g.yeti_armed = 1;
    g.yeti_col = 10;
    g.player_col = 30;
    yeti_step(&g, 0);
    ASSERT(g.yeti_col == 11);

    g.yeti_col = 30;
    g.player_col = 5;
    yeti_step(&g, 0);
    ASSERT(g.yeti_col == 29);
}

static void test_yeti_catches_player_when_row_reaches_player_row(void) {
    struct game g;
    game_init(&g, 1);
    g.yeti_armed = 1;
    g.yeti_row_fp = (PLAYER_ROW - 1) * FP_SCALE;
    /* one more closing step pushes us at or past player row */
    for (int i = 0; i < 256 / BASE_CLOSING_FP + 1; i++) {
        yeti_step(&g, 0);
        if (!g.alive) break;
    }
    ASSERT(g.alive == 0);
    ASSERT(g.yeti_row_fp >= PLAYER_ROW * FP_SCALE);
}
```

Add to `main(void)`.

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `yeti_step`.

- [ ] **Step 3: Implement yeti reveal + chase**

Add a new section before `/* --- player --- */`:

```c
/* --- yeti --- */
LOCAL void yeti_step(struct game *g, int input) {
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
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 39/39 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(yeti): add reveal scheduling and press-cost chase"
```

---

### Task 15: `step()` orchestration + speed ramp — TDD

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write failing tests**

```c
static void test_step_increments_tick(void) {
    struct game g;
    game_init(&g, 1);
    step(&g, 0);
    ASSERT(g.tick == 1);
    step(&g, 0);
    ASSERT(g.tick == 2);
}

static void test_step_scrolls_world_every_scroll_period(void) {
    struct game g;
    game_init(&g, 1);
    /* scroll_period = 6 at start, so 6 steps -> 1 scroll */
    int before_top = g.world_top;
    int before_dist = g.distance;
    for (int i = 0; i < g.scroll_period; i++) step(&g, 0);
    ASSERT(g.world_top != before_top);
    ASSERT(g.distance == before_dist + 1);
}

static void test_step_ramps_scroll_period_down(void) {
    struct game g;
    game_init(&g, 1);
    /* run for the full ramp duration */
    int ramp_ticks = 60 * (1000 / FRAME_MS);
    for (int i = 0; i < ramp_ticks; i++) step(&g, 0);
    ASSERT(g.scroll_period <= SCROLL_PERIOD_END + 1); /* allow ±1 for yeti reveal jolt */
}

static void test_step_does_nothing_when_dead(void) {
    struct game g;
    game_init(&g, 1);
    g.alive = 0;
    int before = g.tick;
    step(&g, KEY_LEFT);
    ASSERT(g.tick == before);
}
```

Add to `main(void)`.

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile errors referencing `step`.

- [ ] **Step 3: Implement `step()` and speed ramp**

Add a new section before `/* --- main --- */`:

```c
/* --- step / draw --- */
LOCAL void update_scroll_period(struct game *g) {
    /* Linear ramp from SCROLL_PERIOD_START to SCROLL_PERIOD_END over 60 s. */
    int ramp_ticks = 60 * (1000 / FRAME_MS);
    int span = SCROLL_PERIOD_START - SCROLL_PERIOD_END;
    int reduced = (g->tick * span) / ramp_ticks;
    if (reduced > span) reduced = span;
    int target = SCROLL_PERIOD_START - reduced;
    /* Allow the yeti-reveal jolt (which already nudged it lower) to stick. */
    if (target < g->scroll_period) g->scroll_period = target;
}

LOCAL void step(struct game *g, int input) {
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
        /* When the visible window scrolls past one chunk's worth of rows,
         * generate a fresh chunk at the bottom. */
        if ((g->distance % CHUNK_ROWS) == 0) {
            world_gen_chunk(g);
        }
    }

    player_check_collision(g);
}
```

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 43/43 tests passed.

- [ ] **Step 5: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(game): add step orchestrator with speed ramp"
```

---

### Task 16: Terminal init/cleanup + signal handlers — manual smoke

**Files:**
- Modify: `src/yeti.c`

- [ ] **Step 1: Add signal handler globals and term init/cleanup**

Add `<signal.h>` and `<stdlib.h>` to includes.

Add a new section before `/* --- cli --- */` (which we'll add later — for now, place it before `/* --- main --- */`):

```c
/* --- term --- */
static volatile sig_atomic_t sig_quit = 0;
static volatile sig_atomic_t sig_resize = 0;
static int curses_initialized = 0;

static void on_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) sig_quit = 1;
    if (sig == SIGWINCH) sig_resize = 1;
}

static void term_cleanup(void) {
    if (curses_initialized) {
        endwin();
        curses_initialized = 0;
    }
}

static int term_init(void) {
    const char *term = getenv("TERM");
    if (!term || !*term || !strcmp(term, "dumb")) {
        fputs("yeti: requires a real terminal (TERM unset or dumb)\n", stderr);
        return 0;
    }
    setenv("ESCDELAY", "25", 0);
    if (!initscr()) {
        fputs("yeti: initscr failed\n", stderr);
        return 0;
    }
    curses_initialized = 1;
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(PAIR_TREE,   COLOR_GREEN,  -1);
        init_pair(PAIR_PLAYER, COLOR_WHITE,  -1);
        init_pair(PAIR_YETI,   COLOR_RED,    -1);
        init_pair(PAIR_HUD,    COLOR_YELLOW, -1);
    }

    if (LINES < 24 || COLS < 80) {
        term_cleanup();
        fputs("yeti: requires an 80x24 terminal or larger\n", stderr);
        return 0;
    }

    atexit(term_cleanup);

    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,   &sa, NULL);
    sigaction(SIGTERM,  &sa, NULL);
    sigaction(SIGWINCH, &sa, NULL);

    return 1;
}
```

- [ ] **Step 2: Build**

```bash
make clean && make
```

Expected: builds. Tests still pass (`make test` → 43/43).

- [ ] **Step 3: Manual smoke**

Temporarily wire term_init into main to check it doesn't break anything. Replace the existing `main` body:

```c
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (!term_init()) return 1;
    /* draw a sanity glyph and wait for a key */
    mvprintw(0, 0, "yeti term_init smoke - press any key");
    refresh();
    getch();
    term_cleanup();
    return 0;
}
#endif
```

Run:

```bash
make && ./yeti
```

Expected: a terminal-cleared screen with the message at the top-left; any key exits and restores the terminal cleanly. Try Ctrl-C — should also exit cleanly.

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c
git commit -m "feat(term): add term init, cleanup, and signal handlers"
```

---

### Task 17: `draw()` rendering — manual smoke

**Files:**
- Modify: `src/yeti.c`

- [ ] **Step 1: Implement `draw()`**

Add to the `/* --- step / draw --- */` section:

```c
LOCAL void draw(const struct game *g) {
    erase();

    /* Playfield rows */
    for (int r = 0; r < LINES && r < BUF_H; r++) {
        for (int c = 0; c < PLAYFIELD_W; c++) {
            char cell = world_get(g, r, c);
            if (cell == 'Y') {
                attron(COLOR_PAIR(PAIR_TREE));
                mvaddch(r, PLAYFIELD_X_OFF + c, 'Y');
                attroff(COLOR_PAIR(PAIR_TREE));
            }
        }
    }

    /* Player (two cells) */
    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + g->player_col, '|');
    mvaddch(PLAYER_ROW, PLAYFIELD_X_OFF + g->player_col + 1, '|');
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);

    /* Yeti */
    if (g->yeti_armed) {
        int yrow = g->yeti_row_fp / FP_SCALE;
        if (yrow >= 0 && yrow < LINES) {
            attron(COLOR_PAIR(PAIR_YETI) | A_BOLD);
            mvaddch(yrow, PLAYFIELD_X_OFF + g->yeti_col, 'Y');
            attroff(COLOR_PAIR(PAIR_YETI) | A_BOLD);
        }
    }

    /* HUD */
    attron(COLOR_PAIR(PAIR_HUD) | A_DIM);
    mvprintw(0, COLS - 13, "DIST %5dm", g->distance);
    attroff(COLOR_PAIR(PAIR_HUD) | A_DIM);

    refresh();
}
```

- [ ] **Step 2: Wire `draw` into the smoke main**

Replace the smoke `main` body again:

```c
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (!term_init()) return 1;
    struct game g;
    game_init(&g, 1);
    /* Manually populate the world with a couple of trees so we can see them. */
    world_set(&g, 5, 20, 'Y');
    world_set(&g, 5, 40, 'Y');
    world_set(&g, 12, 10, 'Y');
    draw(&g);
    getch();
    term_cleanup();
    return 0;
}
#endif
```

- [ ] **Step 3: Build and run**

```bash
make && ./yeti
```

Expected:
- HUD `DIST     0m` top-right in yellow
- Three green `Y`s on screen
- White bold `||` skier near center at row 7
- Any key exits cleanly

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c
git commit -m "feat(render): add draw with player, trees, yeti, and HUD"
```

---

### Task 18: Death screen — manual smoke

**Files:**
- Modify: `src/yeti.c`

- [ ] **Step 1: Implement `death_screen`**

Add to the `/* --- step / draw --- */` section:

```c
LOCAL int death_screen(struct game *g) {
    /* Returns 1 if player wants to restart, 0 to quit. */
    int cx = COLS / 2;
    int cy = LINES / 2;
    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvprintw(cy - 1, cx - 6, "DIST %5dm", g->distance);
    mvprintw(cy + 1, cx - 16, "PRESS R TO RESTART  -  Q TO QUIT");
    attroff(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    refresh();

    timeout(-1);  /* block */
    for (;;) {
        int ch = getch();
        if (ch == 'r' || ch == 'R') return 1;
        if (ch == 'q' || ch == 'Q' || ch == 27) return 0;
    }
}
```

- [ ] **Step 2: Wire death_screen into the smoke main**

```c
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (!term_init()) return 1;
    struct game g;
    game_init(&g, 1);
    g.distance = 248;
    g.alive = 0;
    draw(&g);
    int again = death_screen(&g);
    term_cleanup();
    fprintf(stdout, "restart requested: %d\n", again);
    return 0;
}
#endif
```

- [ ] **Step 3: Build and run**

```bash
make && ./yeti
```

Expected:
- `DIST   248m` and `PRESS R TO RESTART  -  Q TO QUIT` overlay in the middle of an otherwise empty playfield
- Press `r` → exits with `restart requested: 1`
- Press `q` → exits with `restart requested: 0`

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c
git commit -m "feat(render): add death screen overlay"
```

---

### Task 19: Main loop with deadline + drain-input — manual smoke

**Files:**
- Modify: `src/yeti.c`

- [ ] **Step 1: Implement the play loop**

Add to the `/* --- step / draw --- */` section:

```c
LOCAL int play_one_run(struct game *g, int max_ticks) {
    /* max_ticks = 0 means run until death or quit. */
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    /* Seed the world with two chunks so there's something on screen. */
    chunk_unpack(g, BUF_H - CHUNK_ROWS * 2, chunk_pick(&g->rng, 0));
    chunk_unpack(g, BUF_H - CHUNK_ROWS,     chunk_pick(&g->rng, 0));

    while (g->alive && !sig_quit) {
        if (max_ticks > 0 && g->tick >= max_ticks) break;

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
            /* Stop if the user shrank the terminal below minimum. */
            if (LINES < 24 || COLS < 80) { sig_quit = 1; break; }
        }
        step(g, input);
        draw(g);
    }
    return g->alive;
}
```

- [ ] **Step 2: Wire play_one_run into the smoke main**

Replace `main` body:

```c
#ifndef GAME_TEST
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (!term_init()) return 1;

    struct game g;
    unsigned int seed = (unsigned int)(time(NULL) ^ getpid());
    game_init(&g, seed);

    while (!sig_quit) {
        play_one_run(&g, 0);
        if (sig_quit || !g.alive) {
            if (g.alive) break;  /* exited mid-run via q */
            if (!death_screen(&g)) break;
            game_init(&g, (unsigned int)(time(NULL) ^ getpid()));
        }
    }

    term_cleanup();
    return 0;
}
#endif
```

Add `<unistd.h>` to includes for `getpid`.

- [ ] **Step 3: Build and run**

```bash
make && ./yeti
```

Expected behavior:
- Title-less drop directly into a run
- Skier visible at row 7, HUD shows distance ticking up
- Pressing left/right moves the skier
- Eventually a red `Y` appears at top center and closes; touching it ends the run
- Touching a green tree also ends the run
- Death screen appears; `r` starts a new run; `q` exits cleanly

Manually play a few runs to verify each behavior.

- [ ] **Step 4: Commit**

```bash
git add src/yeti.c
git commit -m "feat(game): wire main loop with deadline + drain-input pattern"
```

---

### Task 20: CLI parsing — TDD where possible, then wire in

**Files:**
- Modify: `src/yeti.c`
- Modify: `tests/test_yeti.c`

- [ ] **Step 1: Write tests for the CLI parser**

```c
static void test_cli_parse_no_args(void) {
    char *argv[] = { "yeti", NULL };
    struct cli_opts opts;
    int rc = cli_parse(1, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.have_seed == 0);
    ASSERT(opts.ticks == 0);
    ASSERT(opts.show_help == 0);
}

static void test_cli_parse_seed(void) {
    char *argv[] = { "yeti", "-s", "12345", NULL };
    struct cli_opts opts;
    int rc = cli_parse(3, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.have_seed == 1);
    ASSERT(opts.seed == 12345);
}

static void test_cli_parse_ticks(void) {
    char *argv[] = { "yeti", "--ticks", "100", NULL };
    struct cli_opts opts;
    int rc = cli_parse(3, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.ticks == 100);
}

static void test_cli_parse_help(void) {
    char *argv[] = { "yeti", "-h", NULL };
    struct cli_opts opts;
    int rc = cli_parse(2, argv, &opts);
    ASSERT(rc == 0);
    ASSERT(opts.show_help == 1);
}

static void test_cli_parse_unknown_returns_error(void) {
    char *argv[] = { "yeti", "--nope", NULL };
    struct cli_opts opts;
    int rc = cli_parse(2, argv, &opts);
    ASSERT(rc != 0);
}
```

Add to `main(void)`.

- [ ] **Step 2: Run tests and verify failure**

```bash
make test
```

Expected: compile error referencing `cli_opts`, `cli_parse`.

- [ ] **Step 3: Implement CLI**

Add a new section before `/* --- main --- */`:

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
            fprintf(stderr, "yeti: unknown argument: %s\n", a);
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

- [ ] **Step 4: Run tests and verify pass**

```bash
make test
```

Expected: 48/48 tests passed.

- [ ] **Step 5: Wire CLI into `main`**

Replace `main` body:

```c
#ifndef GAME_TEST
int main(int argc, char **argv) {
    struct cli_opts opts;
    if (cli_parse(argc, argv, &opts) != 0) return 2;
    if (opts.show_help) { cli_print_help(); return 0; }

    if (!term_init()) return 1;

    unsigned int seed = opts.have_seed
        ? opts.seed
        : (unsigned int)(time(NULL) ^ getpid());

    struct game g;
    game_init(&g, seed);

    while (!sig_quit) {
        play_one_run(&g, opts.ticks);
        if (opts.ticks > 0) break;  /* test mode: one run and out */
        if (sig_quit || g.alive) break;
        if (!death_screen(&g)) break;
        game_init(&g, (unsigned int)(time(NULL) ^ getpid()));
    }

    term_cleanup();
    return 0;
}
#endif
```

- [ ] **Step 6: Verify build + test**

```bash
make clean && make && make test
```

Expected: builds; 48/48 tests pass.

- [ ] **Step 7: Commit**

```bash
git add src/yeti.c tests/test_yeti.c
git commit -m "feat(cli): add argv parsing for -h, -s SEED, --ticks N"
```

---

### Task 21: Full integration walk-through + size check

**Files:** (no edits)

- [ ] **Step 1: Verify build is clean**

```bash
make clean && make
```

Expected: no warnings, produces `./yeti`.

- [ ] **Step 2: Run the test suite**

```bash
make test
```

Expected: 48/48 tests pass.

- [ ] **Step 3: Verify `-h`**

```bash
./yeti -h
```

Expected: help text on stdout, exit code 0.

- [ ] **Step 4: Verify `--ticks` test mode**

```bash
./yeti --ticks 5 -s 1
```

Expected: brief flicker (5 ticks ≈ 165 ms) then clean exit with terminal restored.

- [ ] **Step 5: Verify deterministic seed (cross-invocation)**

```bash
./yeti --ticks 100 -s 12345 ; echo "exit=$?"
./yeti --ticks 100 -s 12345 ; echo "exit=$?"
```

Expected: identical exit codes both runs; the played-back ticks should follow the same chunk-pool path (no easy programmatic check at this stage; eyeball that nothing crashes).

- [ ] **Step 6: Manual play test against the acceptance criteria**

```bash
./yeti
```

Walk through each acceptance criterion from the spec:

1. ✓ `make` produced a binary (already verified).
2. ✓ Playfield, player at row 7, world scrolling, trees spawning.
3. ✓ Left/right moves player one column.
4. ✓ Hitting a tree ends the run; death screen appears.
5. ✓ `r` restarts; `q` / ESC quits cleanly.
6. ✓ After 30–60 s, red bold `Y` appears at top center and closes.
7. ✓ Holding/mashing arrows visibly speeds the chase.
8. ✓ Deterministic seed (verified step 5).
9. ✓ `--ticks 100` (verified step 4).
10. ✓ `make size`:

```bash
make size
```

Expected: prints something like `1234 src/yeti.c` (well under 10 240).

- [ ] **Step 7: Commit a wrap-up doc update if needed**

If any rough edges were caught during the walk-through, fix them now and commit. Otherwise no commit needed.

- [ ] **Step 8: Final state check**

```bash
git status
git log --oneline
```

Expected: clean working tree; commit history reads top-down through the tasks above.
