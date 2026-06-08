# Linux Portability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement POSIX `sigaction` signal handling in `src/yeti.c` to achieve persistent signal safety across macOS and Linux (specifically Raspbian 11 armv6l) while maintaining the 10,240-byte source budget constraint.

**Architecture:** Use `sigaction` and `sigemptyset` to register signal handlers for `SIGINT` and `SIGTERM` in `term_init`.

**Tech Stack:** C99, POSIX signals

---

## Plan Tasks

### Task 1: POSIX Sigaction Implementation
Modify signal handler setup in `term_init` to use POSIX `sigaction`.

**Files:**
- Modify: [src/yeti.c:223-228](file:///Users/jonathan/yeti/src/yeti.c#L223-L228)

- [ ] **Step 1: Replace `signal()` calls with `sigaction` block**
  In the `term_init` function, replace the two standard `signal()` registrations with `sigaction` and `sigemptyset`:
  ```c
      struct sigaction sa = { .sa_handler = on_signal };
      sigemptyset(&sa.sa_mask);
      sigaction(SIGINT,  &sa, NULL);
      sigaction(SIGTERM, &sa, NULL);
  ```

- [ ] **Step 2: Clean compile and verify code size**
  Run:
  ```bash
  make clean && make && make size
  ```
  Expected output: Compilation completes cleanly with no warnings, and size is $\le 10240$ bytes.

- [ ] **Step 3: Run manual playthrough**
  Verify the game runs and cleanly captures signals:
  1. Start the game: `./yeti`.
  2. Press Ctrl-C to interrupt.
  3. Expected: Terminal is successfully restored, and the process exits with status 0.

- [ ] **Step 4: Commit changes**
  Stage, commit, and push the updates:
  ```bash
  git add src/yeti.c
  git commit -m "fix(term): use POSIX sigaction for persistent signal handling"
  git push
  ```
