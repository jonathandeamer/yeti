# yeti — byte-budget reduction design

Bring `wc -c src/yeti.c` from 15,854 down to the 10,240-byte advisory ceiling by applying cuts in increasing order of pain, stopping the moment we hit budget.

## Goal

`wc -c src/yeti.c ≤ 10240` while preserving all player-visible behavior.

## Non-goals

- No new game features.
- No change to glyph table, palette, chase mechanic, scroll ramp, or controls.
- No portability beyond macOS for this pass (Linux/BSD support can be restored in a separate session if and when needed).

## Player-visible behavior contract

These behaviors stay identical before and after this work:

- ASCII-only playfield, 80×24 minimum.
- Player skier (`||` white-bold), trees (`T` green), yeti (`Y` red-bold when armed), HUD (`DIST %dm` yellow-dim top-right).
- Vertical scroll with the same speed ramp and yeti-reveal jolt.
- Press-cost chase mechanic with the same `BASE_CLOSING_FP` / `PRESS_COST_FP` tuning.
- Death screen: distance + `PRESS R TO RESTART  -  Q TO QUIT`, blocks for r/q/ESC, respects SIGINT/SIGTERM.
- One-hit collision on tree contact; row-only collision with yeti.
- Reveal in [30 s, 60 s], catch in 5–15 s depending on player input.
- Restart on `r`; quit on `q`/ESC.

## Behind-the-scenes things being removed or simplified

These can be done quietly; nothing the player can observe:

- `-h`, `--help` command-line flag and its help text.
- `-s SEED` and `--ticks N` flags (developer-only knobs).
- `_POSIX_C_SOURCE` feature test (macOS exposes the symbols without it).
- `sigaction` block replaced with three `signal()` calls (BSD signal semantics on macOS are persistent).
- `SIGWINCH` handler and mid-run minimum-size check.
- `atexit(term_cleanup)` belt-and-suspenders cleanup.
- `has_colors()` and `use_default_colors()` guards (macOS Terminal always supports color).
- TERM-unset / TERM=dumb pre-check.
- `initscr()` null-return check and `curses_initialized` flag.
- Position-list comments inside the chunk-pool literal.
- In-function explanatory comments (section dividers stay).
- Trivial wrapper functions whose single call site can absorb the body.
- After every other phase succeeds: `LOCAL` macro + `#ifdef GAME_TEST` guards. The test build is then permanently broken in this repo state; `tests/test_yeti.c` is deleted along with the `test` Makefile target.

## Strategy: iterative, measure-and-decide

Each phase is a single commit. After every phase: `make` (warnings tolerated), `make test` (must pass through phase 7), and `make size`. If `make size` reports ≤ 10240 before reaching phase 7, **stop**. Phases 6 and 7 are the painful ones and are skipped whenever possible.

## Phases

### Phase 1 — Easy wins

No behavior change. Pure deletion.

- Delete the eight `/* chunk N (tier X): ... */` position-list comments inside `chunk_pool`.
- Strip in-function explanatory comments throughout. Keep `/* --- section --- */` dividers and any comment that documents a non-obvious invariant.
- Trim error strings: drop the `yeti: ` prefix, shorten "requires an 80x24 terminal or larger" to "needs 80x24".
- Remove `#define _POSIX_C_SOURCE 200809L`.

Expected saving: **-1,000 B**. Landing: ~14,850.

### Phase 2 — Drop CLI entirely

Remove the entire `/* --- cli --- */` section. `main` reads only `argc` (to know whether to set seed from time or ignore extras) — actually, with no flags, `main` ignores `argv` entirely.

- Delete `struct cli_opts`, `cli_parse`, `cli_print_help`.
- Remove the four `argc/argv`-driven branches from `main`.
- Drop `opts.ticks` from `play_one_run`.

Documentation lives in README only. Unknown args are silently ignored — no parser, no error.

Expected saving: **-1,311 B**. Cumulative: -2,311. Landing: ~13,540.

### Phase 3 — macOS-only term cleanup

- Replace the `sigaction` block + `struct sigaction sa; memset...; sigemptyset...; sigaction(SIGINT...)` etc. with `signal(SIGINT, on_signal); signal(SIGTERM, on_signal);`.
- Drop the `SIGWINCH` handler and the `sig_resize` flag and the mid-run resize check in `play_one_run`.
- Drop `atexit(term_cleanup)` — cleanup happens on the explicit exit paths.
- Drop the `if (has_colors())` guard and `use_default_colors()` call. Use `start_color()` + `init_pair(N, FG, COLOR_BLACK)`.
- Drop the TERM unset/empty/dumb check.
- Drop the `if (!initscr())` failure check.
- Drop the `curses_initialized` flag.

Expected saving: **-500 B**. Cumulative: -2,811. Landing: ~13,040.

### Phase 4 — Re-encode chunks

Replace the 8×4×8 hex-byte grid with eight comma-separated position-list strings, separated by `|` for row boundaries:

```c
static const char *chunks[CHUNK_COUNT] = {
    "27|12,45|40",
    "7,45|25|18,49",
    "18|6,37|26",
    "5,19,38|12,29,46|8,21,45",
    "2,15,36|8,27,47|4,18,36",
    "7,22,41|2,20,45|11,31,48",
    "3,10,22,38,50|6,16,31,46|1,13,23,41,52",
    "6,17,32,45|3,14,33,49|10,24,46,59",
};
static const unsigned char tier[CHUNK_COUNT] = { 0,0,0,1,1,1,2,2 };
```

A row with no trees is encoded as the empty span between two `|`s (or the trailing absence of one). The transition row is implicit — it's always empty. `chunk_unpack` walks the string and writes `'T'` at each comma-separated column, then fills the rest with 0.

Also inline `world_gen_chunk` (a two-liner) at its single call site in `step`.

Expected saving: **-1,400 B**. Cumulative: -4,211. Landing: ~11,640.

### Phase 5 — Inline trivial wrappers

Each of these is called from a single location; inlining the body removes the function header (return type, name, params, braces) without making the caller meaningfully longer:

- `player_lean_input(input)` → `(input == KEY_LEFT || input == KEY_RIGHT)` in `step`.
- `update_scroll_period` → inline its 4 lines in `step`.
- `world_gen_chunk` → already done in phase 4.
- `xorshift_uniform(&s, n)` → `xorshift32(&s) % n` at every call site.
- `add_ms`, `ms_until` → inline math at their two call sites in `play_one_run`.
- `world_clear` → `memset(world, 0, sizeof(world))` in `game_init`.

Expected saving: **-300 B**. Cumulative: -4,511. Landing: ~11,340.

### 🛑 Checkpoint after phase 5

Measure. If `make size` reports a number under 10,240, jump to phase 8. Otherwise, proceed to phase 6.

### Phase 6 — Promote `struct game` to file-scope globals

Only applied if the checkpoint says we're still over.

- Promote each `struct game` field to a file-scope variable (`static unsigned int rng;`, `static int tick;`, etc.).
- Remove the `struct game` definition.
- Remove the `struct game *g` parameter from every function signature.
- Replace every `g->X` access with `X`.
- Remove `&g` from every call site.
- `game_init` becomes a parameterless reset that assigns to globals.

Tests in `tests/test_yeti.c` need updating: each test sets globals directly instead of constructing a struct. (Tests are dropped entirely in phase 8, so this update is short-lived.)

Expected saving: **-600 B**. Cumulative: -5,111. Landing: ~10,740.

### Phase 7 — Light code-golf hot paths

Only applied if phase 6 didn't get us under.

Targeted at `draw`, `step`, `death_screen`, and the `main` loop:

- Combined variable declarations on one line.
- Shorter local names where scope is short and meaning is obvious from context: `r`, `c` for row/col loops; `t` for tick; `p` for player col within a 3-line block.
- Drop one-shot intermediates (e.g. `int range_s = ...; int range_ticks = range_s * ...;` → single expression).
- `?:` ternaries for short branches.

No cryptic global names. No combining unrelated logic into one mega-function. The goal is "denser" not "obfuscated".

Expected saving: **-500 B**. Cumulative: -5,611. Landing: ~10,240.

### Phase 8 — Drop test scaffolding

Final phase, always run:

- Delete `#ifdef GAME_TEST / #else / #endif` and the `LOCAL` macro.
- Delete the `#ifndef GAME_TEST` guard around `main`.
- Restore `static` on every previously-`LOCAL` function and global.
- Delete `tests/test_yeti.c`.
- Remove the `test` and `tests/test_yeti` targets from the `Makefile`.
- Remove `-Wno-missing-prototypes` from the (now-deleted) test build line.

Expected saving: **-90 B**. Cumulative: -5,701. Landing: ~10,150.

## Verification approach

- After every phase 1–7 commit: `make clean && make && make test` must succeed.
- After phase 4: in addition to unit tests, manually run `./yeti -s` removed (no -s anymore) — just `./yeti` and play a few seconds to confirm the new chunk-encoding renders correctly.
- After phase 6 (if applied): test rewrites land in the same commit as the globals refactor; tests must still pass.
- After phase 7 (if applied): same.
- After phase 8: `make` succeeds; the test target is gone, so we skip `make test`. Manual playthrough confirms no behavior change.
- Final `make size` must report ≤ 10240 bytes.

## Out of scope

- Rebuilding the test infrastructure as PTY/integration tests. The current unit-test suite is sacrificed at phase 8; rebuilding tests is a separate session.
- Adding back the dropped CLI flags (`-h`, `-s`, `--ticks`) — separate session if needed.
- Restoring portability beyond macOS — separate session.
- Any new gameplay features.

## Acceptance criteria

1. `make` builds cleanly on macOS (warnings tolerated).
2. `make size` reports a number ≤ 10240.
3. Manual playthrough on a real terminal: skier moves, world scrolls, trees appear at expected density, yeti reveals in 30–60 s and chases, press-cost mechanic is visible, tree contact ends the run, death screen appears, `r` restarts, `q`/ESC quits cleanly, Ctrl-C during death screen exits cleanly.
4. Same playthrough confirms identical visual layout: HUD position, glyph table, colors.
5. The phases not used (any of 6 or 7) stay un-applied — they were only present in case the budget wasn't met without them.
