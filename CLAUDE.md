# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build, run, measure

```sh
make            # build ./yeti (C99 + ncurses, via pkg-config or -lncurses)
make size       # wc -c src/yeti.c — must stay ≤ 10240 bytes
make clean
./yeti          # run; Left/Right to lean, R to restart, Q/ESC to quit
```

There is no test target. The unit-test scaffolding (`LOCAL` macro, `GAME_TEST` guards, `tests/`) was deliberately removed in the byte-budget reduction; do not reintroduce it without an explicit ask. Verification is now: `make`, then manual playthrough in a real terminal.

## Demo recording

`demo/yeti.tape` is a Charmbracelet VHS script that produces `demo/yeti.gif`. Re-record with `vhs demo/yeti.tape`. The recorded run scripts inputs, lets the player die once to show the death screen, then restarts.

## Architecture

The entire game lives in **`src/yeti.c`** as a single file under a self-imposed 10,240-byte source ceiling. This constraint is load-bearing — it drives most non-obvious choices in the code. Before adding anything, check `make size` headroom.

Sections in `src/yeti.c`, top to bottom, marked by `/* --- name --- */` dividers:

- **constants** — frame timing (`FRAME_MS=33`), playfield geometry, fixed-point scale, chase tuning (`BASE_CLOSING_FP`, `PRESS_COST_FP`), scroll-period ramp endpoints, color pair IDs.
- **state** — `struct game` holds everything: RNG, tick, distance, player col, alive flag, yeti reveal/position state, scroll ramp counters, and a `BUF_H × PLAYFIELD_W` ring buffer plus its `world_top` cursor.
- **time** — `add_ms` / `ms_until` for the monotonic deadline loop.
- **prng** — xorshift32; `xorshift_uniform(s, n)` is `xorshift32 % n` (biased, acceptable here).
- **world** — `chunks[]` is the obstacle pool, encoded as pipe-separated position-list strings (`"col,col|col|col,col,col"`), three rows of trees plus an implicit empty transition row. `chunk_tier[]` maps each chunk to difficulty tier 0/1/2. `chunk_pick` selects by distance band; `chunk_unpack` decodes a string into the ring buffer at a given destination row.
- **player** — column-only movement; collision checks both columns of the 2-wide `||` skier against `'T'` on `PLAYER_ROW`.
- **yeti** — armed at a randomized reveal tick in [30 s, 60 s]. Once armed, advances row in fixed-point (`FP_SCALE=256`); every Left/Right press adds `PRESS_COST_FP` to its advance — this is the press-cost chase mechanic. Catches the player when its row reaches `PLAYER_ROW * FP_SCALE`.
- **term** — ncurses init, color pairs, signal handlers (`SIGINT`/`SIGTERM` set `sig_quit`). macOS-targeted: relies on persistent BSD `signal()` semantics.
- **step / draw** — `step` advances tick, updates scroll period via the ramp, moves player and yeti, scrolls world and spawns a new chunk every `CHUNK_ROWS` rows of scroll, then checks collision. `draw` renders trees, player, yeti, and the `DIST %dm` HUD.
- **main loop** — `play_one_run` uses a deadline-based frame clock: compute next deadline, drain all pending input until then (the last keystroke wins as the step's input), step, draw. `death_screen` polls with a 200 ms timeout so SIGINT during the post-death wait still exits cleanly. `main` is a restart loop around `play_one_run` and `death_screen`.

### Invariants worth knowing before editing

- `world_top` indexes the ring buffer modulo `BUF_H`. Always go through `world_get` / `world_set` — they handle the modulo. `BUF_H` must be ≥ `CHUNK_ROWS + PLAYER_ROW + slack` and is currently 32.
- Chunk strings are decoded into rows 0..N-1 of `CHUNK_ROWS`; the final row stays empty as the transition spacer between successive chunks. Don't put a trailing `|`-terminated row in a chunk string expecting it to render — `chunk_unpack` pre-clears the destination rows.
- The scroll-period ramp in `update_scroll_period` is monotonically non-increasing: it only ever lowers `scroll_period`. The yeti-reveal jolt (`scroll_period -= 1` once when armed) is preserved because the ramp's `target < scroll_period` guard won't undo it.
- Fixed-point arithmetic: `FP_SCALE = 256`. The yeti row is stored in fixed-point; convert with `/ FP_SCALE` only at draw time.

## Project rules

- **Single-file ceiling.** `src/yeti.c` must stay ≤ 10240 bytes (`make size`). If a change pushes over, either compress elsewhere first or revisit `docs/superpowers/specs/2026-05-25-yeti-byte-budget-design.md` for the contract about what's allowed to be dropped.
- **macOS-only assumptions are in the code.** Persistent BSD `signal()` semantics, no SIGWINCH handler, no defensive `has_colors()` check. Restoring Linux/BSD portability is a separate session; don't sprinkle ifdefs piecemeal.
- **No version numbers in repo artifacts.** "v0.1", "v1.0" etc. are chat shorthand only — never in code, commit messages, manpage, or docs.
- **Yeti docs are self-contained.** Do not reference external design notes (e.g. `~/fresh`) in anything inside this repo.
- **Forbidden-sources discipline is binding on AI assistants.** See `FORBIDDEN-SOURCES.md`. No source contact with SkiFree, ESR's `ski`, the Fortran skiing lineage, `asciijump`, or any other ski-game source/decomp/post-mortem. If a forbidden source is encountered, stop reading, log it, quarantine the affected area.

## Commit conventions

Conventional Commits with one of these scopes (a git hook enforces them):

```
game world yeti render term score cli build docs ci meta
```

Subject ≤ 72 chars, lowercase, no trailing period. Body explains *why*. The `cli` scope persists in the allow-list even though the CLI was removed.

## Design docs

Design specs and implementation plans live under `docs/superpowers/{specs,plans}/`, dated `YYYY-MM-DD-`. The two canonical docs are:

- `specs/2026-05-25-yeti-minimum-playable-design.md` — original gameplay contract (glyph table, palette, chase mechanic, scroll ramp, controls, death screen).
- `specs/2026-05-25-yeti-byte-budget-design.md` — the player-visible behavior contract that the source-size reduction had to preserve, plus what was deliberately removed (CLI, SIGWINCH, atexit cleanup, test scaffolding).

These are point-in-time snapshots from when each piece of work was planned, not a living spec. Behavior has been tuned and improvised since (terrain density, skier/HUD colors, distance label, etc.); treat the docs as historical context, and the current `src/yeti.c` as the source of truth for how the game actually behaves today.
