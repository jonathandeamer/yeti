# Death-screen resize redraw — design

Date: 2026-06-19
Issue: #3 — "Fix missing redraw and layout breakage during terminal resize on the game over screen"

## Problem

`death_screen` draws the game-over overlay once, then blocks in a `getch()`
input loop waiting for `r`/`q`. The loop has no redraw logic. When the terminal
is resized, `getch()` returns `KEY_RESIZE` and ncurses updates `LINES`/`COLS`,
but the screen is never cleared or redrawn, so the layout garbles.

## Scope

Core redraw only (decided against the byte budget — `make size` headroom was
22 bytes at design time, 10218/10240).

Explicitly **out of scope**, deferred for budget reasons:
- Clamping the overlay x-coordinates (`cx - 16`) so the restart line still
  draws on terminals narrower than 32 columns.
- An active minimum-size guard during a run (would reopen the deliberately
  removed SIGWINCH territory).

## Approach

Restructure `death_screen` so its rendering is repeatable and re-runs on
`KEY_RESIZE`, reusing the existing `draw(g)` as the clear-and-redraw primitive.
`draw(g)` already does `erase()` → redraw trees/player/yeti/HUD → `refresh()`,
so it rebuilds the final frame with no new rendering code.

```c
static int death_screen(const struct game *g) {
    int cx = PLAYFIELD_X_OFF + PLAYFIELD_W / 2, cy = 12;
redraw:
    draw(g);
    attron(COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    mvprintw(cy - 1, cx - 8,  "YOU SKIED %5dm", g->distance);
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
```

### Rationale

- `goto redraw` keeps the original flat indentation, so the change costs
  ~10 bytes. Wrapping the body in an extra loop nest would blow the budget on
  re-indentation alone.
- The old `if (ch == ERR) continue;` is dropped: `ERR` now falls through and
  re-polls (the 200 ms `timeout` still paces the loop), while `KEY_RESIZE`
  jumps to a full redraw.
- `cy` is collapsed from `24 / 2` to `12` and merged onto the `cx` line to
  reclaim bytes. `cx`/`cy` are fixed playfield-geometry constants, not derived
  from `COLS`/`LINES`, so per-resize recomputation is a no-op — the redraw is
  what fixes the garbling.

## Verification

- `make` builds clean.
- `make size` stays ≤ 10240.
- Manual: run `./yeti`, die, resize the terminal on the death screen, confirm
  the frame re-centers and is not garbled.
