# Rules of the Slope (Contributing to yeti)

Welcome to the slopes! `yeti` is a minimalist, retro terminal skiing game. Contributions are welcome, but to keep the snow fresh and the mountain run fast, please play by a few simple rules.

---

### 🎒 Rule 1: Pack Light (The 10 KiB Budget)
The entire game lives in one C file: [src/yeti.c](file:///Users/jonathan/yeti/src/yeti.c). 

To keep the game minimalist, portable, and lightweight, the source file is strictly capped at a self-imposed **10,240-byte budget** (10 KiB). 
*   Before submitting a pull request, run `make size` to check your code size.
*   If your changes push the file size over 10,240 bytes, you will need to do some code-golfing or refactor existing parts to squeeze your features in!

### 📝 Rule 2: Sign the Logbook (Conventional Commits)
Conventional Commits format is enforced to keep history clean. 
*   **Set up the commit hook locally first:** Run `make init-hooks` in your terminal. This will link the commit message validator.
*   **Format:** `type(scope): short description` (e.g., `fix(render): center death screen`).
*   Allowed scopes are listed in [README.md](file:///Users/jonathan/yeti/README.md) and [CLAUDE.md](file:///Users/jonathan/yeti/CLAUDE.md).

### ❄️ Rule 3: Original Snow Only (Fresh Code & AI)
Please write code independently. Both human and AI-assisted contributions are welcome as long as they are tested thoroughly by the contributor and kept entirely free from reference to external codebases. Do not look at or reference other skiing game sources (such as the classic *SkiFree* or ESR's *ski*) when writing code.

### 🎿 Rule 4: Upgrades & Obstacles (Feature Ideas)
Got a cool idea for a feature or obstacle? Contributions are welcome!
*   **Discuss on an Issue first:** Please open an issue to discuss new features before writing any code.
*   **Keep it cozy:** Remember the 10 KiB limit. If you can fit a new obstacle or feature into the code under the size ceiling, it's fair game!

---

Thanks for helping keep the yeti moving! See you on the run.
