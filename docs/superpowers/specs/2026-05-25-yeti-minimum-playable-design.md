# yeti — minimum-playable design

The smallest binary that's recognizably the game: a single-file C + ncurses ski-down where the yeti is the inevitable end of every run.

## What this document is for

The implementation contract for the first playable. It exists so the implementation plan that follows can be derived mechanically — no decision should require coming back to brainstorm. Anything genuinely open is called out under "Tunable constants" or "Out of scope."

## Goal in one sentence

Launch the binary, ski down a snowy mountain, dodge trees, eventually be eaten by the yeti, hit `r`, do it again.

## Identity locks

- **Genre tag (taxonomy):** endless scroller. Not the player-facing positioning — see "Help text."
- **Scroll axis:** vertical, top-to-bottom. The world scrolls upward toward the player. The player sits at a fixed row in the upper third of the screen.
- **Failure model:** one-hit-and-done. Tree contact ends the run. Yeti contact ends the run.
- **The yeti is inevitable.** There is no win condition. Every run ends in being caught.
- **Single mode.** No slalom, no tricks, no power-ups, no leaderboards, no audio system.
- **Keyboard-only, ASCII-only, no mouse.**
- **Source-file byte budget:** `wc -c src/yeti.c` ≤ 10 240 bytes (advisory at this stage; CI gate later).

## Source-contact discipline

`yeti` is implemented independently of any predecessor or sibling ski game. The discipline that binds this — for human contributors and AI assistants alike — is in `FORBIDDEN-SOURCES.md` at the repo root. Every participant in the project is bound by it.

## Architecture

Single translation unit `src/yeti.c`, read top-down with section-comment dividers:

1. `/* --- includes --- */`
2. `/* --- constants --- */`
3. `/* --- prng --- */` (xorshift32)
4. `/* --- world --- */` (chunk pool + step + accessor)
5. `/* --- player --- */`
6. `/* --- yeti --- */`
7. `/* --- step / draw --- */`
8. `/* --- term --- */` (curses init/cleanup, signal install)
9. `/* --- cli --- */`
10. `/* --- main --- */`

One state struct, `struct game`, threaded through every function that mutates state. No scattered globals beyond signal flags.

```c
struct game {
    unsigned int rng;            /* xorshift32 state */
    int tick;                    /* monotonic; drives reveal scheduling */
    int distance;                /* rows scrolled total; HUD displays this */
    int player_col;              /* 0..PLAYFIELD_W - 2 (player is 2 cells wide) */
    int alive;                   /* 0 = dead, fall through to death screen */
    int yeti_armed;              /* 0 pre-reveal, 1 chasing */
    int yeti_reveal_tick;        /* tick at which armed flips to 1 */
    int yeti_row_fp;             /* fixed-point screen row * FP_SCALE */
    int yeti_col;                /* 0..PLAYFIELD_W - 1 */
    int scroll_period;           /* ticks per row of world scroll; ramps down */
    int ticks_since_scroll;      /* mod scroll_period */
    char world[BUF_H][PLAYFIELD_W]; /* ring buffer of rows; cells are 0 or 'Y' */
    int world_top;               /* index of topmost row in the ring */
};
```

## Constants reference

Non-tunable (locked by the design):

- `FRAME_MS = 33` — ~30 fps target
- `PLAYFIELD_W = 60` — usable playfield width in cells
- `PLAYFIELD_X_OFF = 10` — left margin; `(80 - 60) / 2`
- `PLAYER_ROW = 7` — fixed screen row of the skier
- `BUF_H = 32` — world ring-buffer height (≥ terminal LINES + a chunk's worth)
- `FP_SCALE = 256` — fixed-point scale for `yeti_row_fp` and closing math
- `CHUNK_COUNT = 8`, `CHUNK_ROWS = 4`, `CHUNK_BYTES = 8` — chunk-pool dimensions

See "Tunable constants" below for first-pass values slated for playtest tuning.

## Main loop and frame timing

`FRAME_MS = 33` (~ 30 fps). The main loop computes an absolute next-frame deadline using `clock_gettime(CLOCK_MONOTONIC)` and uses ncurses `timeout()` + `getch()` to drain input until the deadline. Input bursts and held keys cannot make the loop iterate faster than `FRAME_MS` because the deadline math is absolute, not anchored to `now`.

```c
struct timespec next;
clock_gettime(CLOCK_MONOTONIC, &next);

while (g.alive && !sig_quit) {
    add_ms(&next, FRAME_MS);
    int input = 0;
    for (;;) {
        int ms_left = ms_until(next);
        timeout(ms_left > 0 ? ms_left : 0);
        int ch = getch();
        if (ch == ERR) break;        /* deadline reached */
        input = ch;                  /* latest key wins */
    }
    if (sig_resize) { handle_resize(&g); sig_resize = 0; }
    step(&g, input);
    draw(&g);
}
```

Each frame: compute deadline → drain input → step one tick → draw.

`atexit(term_cleanup)` is the safety net for normal exit only. It does **not** catch `abort()`, fatal signals beyond the ones we handle, or `_exit()`. Signal-driven exits flow through the main loop's `sig_quit` check, which falls through to explicit cleanup.

`SIGINT` / `SIGTERM` set `sig_quit`. `SIGWINCH` sets `sig_resize`, handled at the top of the next frame. Signal handlers do async-signal-safe writes only (`sig_atomic_t` flags).

## World generation — chunk pool, no runtime solver

A small static pool of hand-designed 4-row × 60-column chunks. The pool is data, not code:

```c
#define CHUNK_COUNT  8
#define CHUNK_ROWS   4
#define CHUNK_BYTES  8                /* 60 cols rounded up to 64 bits */
static const uint8_t chunk_pool[CHUNK_COUNT][CHUNK_ROWS][CHUNK_BYTES] = { ... };
static const uint8_t chunk_tier[CHUNK_COUNT] = { 0, 0, 0, 1, 1, 1, 2, 2 };
```

Each chunk has 3 active rows + 1 always-empty transition row (the last row). Bitmap encoding: 8 bytes per row × 4 rows × 8 chunks = 256 bytes of static data, plus an 8-byte tier table. Top 4 bits of each row byte are unused (60 columns, 64 bits available).

**Hand-verified invariants** (checked at design time, not at runtime):

- No run of 3+ consecutive trees in any row → player can always sidestep with a one-column move.
- Between any two adjacent non-empty rows of a chunk, at least one column-step transition is tree-free.
- The last row is empty → chunk-to-chunk boundaries are always passable from any column.

Because the transition row is empty, chunks compose freely without a runtime reachability solver. The pool itself is the guarantee.

**Density ramp**: tier 0 chunks (sparse, ~1–2 trees per non-empty row) are pool indices 0–2; tier 1 (medium, ~3 trees per row) are 3–5; tier 2 (dense, ~4–5 trees per row) are 6–7. Generator picks based on `g.distance`:

- `0 ≤ distance < 100` → tier 0 only
- `100 ≤ distance < 200` → tier 0 or 1 (50/50)
- `200 ≤ distance` → tier 1 or 2 (50/50)

Within a tier, uniform-random pick via xorshift32.

**World ring buffer**: `BUF_H = 32`. New chunks are unpacked from the bit-packed `chunk_pool` into the ring buffer at generation time (`'Y'` for set bits, `0` for clear). World scroll advances `world_top` upward modulo `BUF_H`. The ring is byte-per-cell for direct indexing in `world_get()` and the draw loop; bit-packing the runtime buffer would save ~1.7 KB of stack and cost more bytes in code than it returns.

## Player input and movement

| Key | Action |
|---|---|
| Left arrow | `player_col--` (clamped at 0) |
| Right arrow | `player_col++` (clamped at PLAYFIELD_W − 2) |
| `q`, ESC | clean exit |
| `r` | ignored mid-run; restarts from death screen |

One column per tick on lean input. Player is two cells wide (`||`); the right cell sits at `player_col + 1`. With ~30 fps and a 60-column playfield, a full-width sweep takes ~2 s.

## Speed ramp

`scroll_period` (ticks per row of world scroll) starts at 6 ticks/row (~5 rows/s) and ramps down to 3 ticks/row (~10 rows/s) over the first 60 s of the run. Implementation: linear interpolation on `g.tick`, clamped to a hard minimum.

At yeti reveal, drop `scroll_period` by 1 immediately — the visible "now it's serious" jolt. Cosmetic only; the chase math is independent of scroll speed.

## Yeti reveal and chase — press-cost mechanic

**Reveal scheduling**: at `game_init`,

```c
g.yeti_reveal_tick = (REVEAL_MIN_S + xorshift_uniform(REVEAL_MAX_S - REVEAL_MIN_S))
                   * (1000 / FRAME_MS);
```

With `FRAME_MS = 33`, that's a uniform-random tick in roughly `[~900, ~1800]`.

**Pre-reveal** (`yeti_armed == 0`): yeti is not drawn, no chase math.

**At reveal** (`g.tick == yeti_reveal_tick`): set `yeti_armed = 1`, `yeti_row_fp = 0`, `yeti_col = PLAYFIELD_W / 2`, drop `scroll_period` by 1.

**Chase tick** (`yeti_armed == 1`):

```c
g.yeti_row_fp += BASE_CLOSING_FP;
if (lean_input_this_tick) g.yeti_row_fp += PRESS_COST_FP;
if (g.yeti_col < g.player_col) g.yeti_col++;
else if (g.yeti_col > g.player_col) g.yeti_col--;
if (g.yeti_row_fp >= PLAYER_ROW * FP_SCALE) g.alive = 0;
```

Linear per-press cost, no decay accumulator. Yeti horizontal drift is purely cosmetic (1 col/tick toward player) — collision is row-only.

**Time-to-catch envelope** with `BASE_CLOSING_FP = 5`, `PRESS_COST_FP = 30`, `FP_SCALE = 256`, `PLAYER_ROW = 7` (gap = 1792 fp):

| Player behavior | Approximate seconds |
|---|---|
| No presses | ~12 s |
| Light dodging (1 press / 10 ticks) | ~7.5 s |
| Heavy dodging (1 press / 3 ticks) | ~4 s |

Constants are tunable.

## Collision

Once per `step()`, after world scroll and yeti chase update:

```c
char left  = world_get(&g, PLAYER_ROW, g.player_col);
char right = world_get(&g, PLAYER_ROW, g.player_col + 1);
if (left == 'Y' || right == 'Y') g.alive = 0;
```

Yeti collision is checked inside the chase block. Same outcome for both: `alive = 0`, drop into the death screen.

## Rendering

**Layout** (80×24 minimum; smaller terminal → message and clean exit):

- Rows 0..LINES − 1 (≥ 24)
- Playfield occupies columns 10..69 (60 wide)
- HUD on row 0, right-aligned: `mvprintw(0, COLS - 13, "DIST %5dm", g.distance)`
- Player at row 7

**Glyph table** (ASCII):

| Glyph | Color pair | Meaning |
|---|---|---|
| `Y` | green | tree |
| `Y` | red bold | yeti (only when armed) |
| `\|\|` (2 cells) | white bright | player skier |
| space | — | empty playfield |
| `DIST %dm` | yellow dim | HUD |

Four `init_pair()` calls. `use_default_colors()` so background is whatever the user's terminal has — sidesteps the COLOR_BLACK contrast issue. Monochrome terminals get shape-only reads; the game stays playable.

**Draw order per frame:**

1. `erase()`
2. For each visible row, write 60 playfield cells from the world ring buffer.
3. Write the two player cells.
4. If `yeti_armed`, write the yeti.
5. Write the HUD.
6. `refresh()`.

## Death screen

When `alive == 0`, leave the last frame on screen and overlay text dead-center:

```
DIST 248m

PRESS R TO RESTART  -  Q TO QUIT
```

Block on `getch()` (no timeout). `r`/`R` → re-init the game with a fresh `time(NULL) ^ getpid()` seed (the `-s` flag, if passed, applies only to the first run of the session). `q`/`Q`/ESC → clean exit via `term_cleanup` + `return 0`.

## Terminal init and cleanup

```c
setenv("ESCDELAY", "25", 0);   /* 25 ms ESC responsiveness */
initscr();
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
atexit(term_cleanup);
```

`term_cleanup` calls `endwin()` if curses was initialized. Signal handlers use `sigaction` (not `signal`) and set `sig_atomic_t` flags.

`TERM` unset, empty, or `dumb` → one-line stderr message + exit before `initscr()`.

## CLI

Manual `argv` walk, no `getopt`. Flags:

- `-h`, `--help` → help to stdout, exit 0.
- `-s SEED` → parse as `unsigned long` (`strtoul`), use as initial RNG seed instead of `time(NULL) ^ getpid()`.
- `--ticks N` → run for N ticks total then exit cleanly (test seam; not in help).

**Help text:**

```
yeti - ski down a mountain until the yeti finds you

USAGE
    yeti [-s SEED]

OPTIONS
    -h, --help        show this help and exit
    -s SEED           set RNG seed for a deterministic run

CONTROLS
    LEFT / RIGHT      lean
    Q, ESC            quit
    R                 restart from death
```

No version line.

## Tunable constants

First-pass guesses, slated for playtest-driven tuning:

- `BASE_CLOSING_FP = 5` — yeti baseline closing speed per tick (fixed-point)
- `PRESS_COST_FP  = 30` — yeti closing bonus per lean press (fixed-point)
- `scroll_period` start = 6, end = 3
- `REVEAL_MIN_S = 30`, `REVEAL_MAX_S = 60`
- Density-tier thresholds: 100, 200 distance units

## Build

`Makefile` honoring `CC`, `CFLAGS`, `LDFLAGS`, `DESTDIR`, `PREFIX`. ncurses via `pkg-config --cflags --libs ncurses`, fall back to bare `-lncurses` if pkg-config is absent.

Warning baseline: `-std=c99 -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition -fno-common`. Local builds warn; CI later adds `-Werror`.

Targets:

- `all` → `yeti`
- `clean`
- `install` / `uninstall` honoring `DESTDIR` + `PREFIX`
- `size` → `wc -c src/yeti.c`

## Repo layout after this session

```
yeti/
├── .git/
├── .gitignore                                              (just: yeti)
├── FORBIDDEN-SOURCES.md                                    (already committed)
├── LICENSE                                                 (root pointer to LICENSES/)
├── LICENSES/
│   ├── MIT.txt
│   └── CC0-1.0.txt
├── Makefile
├── README.md                                               (minimal: what it is, build, controls, license)
├── src/
│   └── yeti.c
└── docs/
    └── superpowers/
        └── specs/
            └── 2026-05-25-yeti-minimum-playable-design.md   (this file)
```

**Licensing**: dual `MIT OR CC0-1.0`. REUSE-compliant `LICENSES/` directory holding both texts; root `LICENSE` is a short pointer. SPDX header in every source file: `// SPDX-License-Identifier: MIT OR CC0-1.0`. README mentions the dual license in one sentence.

**Git-hook tracking** is deferred — the commit-msg hook lives in `.git/hooks/` (untracked) and is enforced for the original author. Moving it to a tracked location is a small follow-up when contributors arrive.

## Out of scope for this session

These belong to follow-up sessions, each with its own design decisions:

- Skier lean variants (`//`, `\\`) — lean-cancel behavior must be locked
- Rocks (second hazard) — contrast attribute decision (`COLOR_BLACK + A_DIM` is invisible on black backgrounds)
- High-score persistence (XDG state path + atomic write)
- BEST display in HUD
- Yeti reveal flash frame
- Death tableau (two-frame sequence)
- Trail fade
- Title screen
- First-run controls hint
- ESC pause overlay
- `?` help overlay
- Daily-seed mode
- Manpage (`yeti.6`)
- CI matrix
- Hardening flags on install builds (`-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`, `-fPIE`, `-pie`)
- Sanitizer builds for dev
- PTY tests via Python + pexpect

## Acceptance criteria

The session is done when:

1. `make` produces a `yeti` binary using `pkg-config`-detected ncurses (or `-lncurses` fallback).
2. Running `yeti` shows a playfield with the player at row 7, the world scrolling, trees spawning from the chunk pool.
3. Pressing left/right moves the player one column.
4. Hitting a tree ends the run; the death screen appears.
5. Pressing `r` starts a fresh run; `q` or ESC quits cleanly with the terminal restored.
6. After 30–60 s of running, a red bold `Y` appears at the top center and closes on the player.
7. The yeti's closing rate is visibly faster when the player presses left/right than when they hold straight.
8. `yeti -s 12345` produces a deterministic run (same chunk sequence, same reveal tick) — repeatable across invocations and across machines, since game state advances by tick rather than wall-clock.
9. `yeti --ticks 100` runs 100 ticks and exits cleanly.
10. `make size` reports the source-byte count.
