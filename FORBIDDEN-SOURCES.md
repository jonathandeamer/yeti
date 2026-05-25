# FORBIDDEN SOURCES

The source-contact discipline for `yeti`. This file records the project's commitment to independent reimplementation, and is binding on every participant — human and AI.

## Purpose

`yeti` is implemented independently of any predecessor or sibling ski game. This document exists so that:

- The rule survives the original author's memory.
- AI assistants brought into the project have a clear, explicit contract.
- Future contributors see the boundary before their first patch.
- Downstream packagers and legal reviewers can verify the project's provenance.
- The audit trail outlives any individual collaborator.

## The rule

**No source contact with any predecessor or sibling ski game, in any form, by anyone.**

This is stricter than clean-room reverse engineering: there is no "clean room" half of the team that reads the originals. Nobody on the project reads ski-game source, ever.

## What counts as source contact

**Forbidden:**

- Source code in any language.
- Decompilations and disassemblies.
- Implementation post-mortems that quote or paraphrase code.
- Technical analyses that explain algorithms via pseudocode taken from a specific implementation.
- Bug reports, issues, or patches that include code snippets from a forbidden source.
- Forks, ports, clones, and derivatives of any forbidden source.
- Reading any of the above inside a debugger, hex editor, or `strings` dump.

**Allowed:**

- Gameplay descriptions (text).
- Screenshots and box art.
- Videos, let's plays, speed runs.
- Reviews and retrospectives (text).
- General design analyses that do not quote source.
- Memory of having played a predecessor.
- General ncurses, POSIX, and C-language documentation.
- This project's own design notes and discussions.
- Non-ski terminal arcade games used as craft references, **without reading their source**.

## Specifically forbidden sources

### Ski-game lineage

- **SkiFree** (Chris Pirih, Microsoft Entertainment Pack 3, 1991) — direct gameplay inspiration. The abandonware copies and various ports floating around the web are off-limits.
- **The Fortran "Skiing" lineage** — a family of Fortran ASCII ski games from 1970s–80s academic computing. Authorship is fuzzy in the historical record; treat the entire lineage as forbidden rather than trying to pin down individual authors. Do not seek out source archives, BBS-era code dumps, or vintage-computing repos that host them.
- **`ski`** by Eric S. Raymond (BSD-2-Clause, distributed via `catb.org/~esr/ski/`) — the nearest sibling in the terminal/ncurses space, packaged on Homebrew and AUR. This is the source most likely to be accidentally encountered. Treat with maximum caution.
- **`asciijump`** — adjacent terminal ASCII ski-jump game distributed in Debian. Different sub-genre but ski-game lineage.
- **Any other ski-game clone, fork, port, or derivative** — by default, anything in the ski-game lineage is forbidden. When in doubt, treat as forbidden.

### Decompilation and implementation analysis

Decompilations, disassemblies, and technical implementation post-mortems of any forbidden source are also off-limits, even when hosted on otherwise reputable venues (retro-game blogs, academic papers, vintage-computing forums, conference talks). The rule binds the *content*, not the venue.

### Adjacent projects (limited restriction)

- **`nbsdgames`** (CC0) — general reference is **allowed** for packaging conventions, portability patterns, and the small-Unix-game craft tradition. Reading the source of any individual game in the collection to inform `yeti`'s logic is **forbidden**. The boundary is: study how the collection ships; do not study how any individual game implements its mechanics.

## Slip-up protocol

Source can be encountered by accident — a search result, a stray click, an autoplaying video that pulls up code, an AI tool that surfaces a snippet. When it happens:

1. **Stop reading immediately.** Do not try to "see more" to figure out whether it was bad. Close the tab, kill the buffer, abandon the terminal.
2. **Log the incident** in the "Incident log" section below: date, what was seen, how it was encountered, what (if anything) was learned.
3. **Quarantine the affected area.** Do not write code in the system the forbidden source informed for a cooling-off period — at minimum days, probably weeks. Use the time to work on unrelated parts of the project.
4. **Do not try to "unsee."** That is impossible. The audit trail must reflect reality. A documented slip-up is recoverable; a hidden one is not.

## AI assistants and this discipline

AI tools used on this project are bound by the same rule as humans.

- AI receives gameplay descriptions, screenshots, videos, and design writing — not predecessor source code, decompilations, or implementation post-mortems.
- Web search and fetch tools driven by AI must not be pointed at the forbidden sources above. Where ambiguous (e.g., a package-manager page that links to upstream source), AI fetches the description page only and does not click through to the source repository.
- An AI assistant that retrieves a forbidden source — by mistake or by being misdirected — triggers the slip-up protocol for the affected session and for any code that session produced.
- This discipline applies regardless of which AI vendor, model, or interface is used.

## Incident log

*(None yet. Append dated entries here if a forbidden source is ever encountered.)*
