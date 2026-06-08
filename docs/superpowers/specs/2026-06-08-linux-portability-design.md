# yeti — Linux Portability Design

Restore robust signal handling portability to `yeti` (specifically targeting Raspbian 11 Bullseye on armv6l) while maintaining the 10,240-byte source budget constraint.

## Goal

Achieve reliable, persistent signal handling on Linux systems without platform-conditional `#ifdef` compilation flags.

### Out of Scope
* `SIGWINCH` resize handling (omitted per byte-budget specs).
* `has_colors()` terminal guards (assumed color support present).

## Approach: POSIX `sigaction`

Standard C99 `signal()` has undefined persistence semantics on POSIX platforms, meaning the handler can be reset to `SIG_DFL` after the first trigger. On macOS (BSD), it remains persistent by default. 

To ensure persistent signal handling on Linux (glibc/musl) and macOS, we will use POSIX `sigaction`.

### Strict POSIX Compliance (`sigemptyset`)
Although zero-initializing `sa_mask` (via `struct sigaction sa = {0}`) works on macOS and Linux glibc/musl, `sigset_t` is technically an opaque type. For strict portability, we explicitly call `sigemptyset(&sa.sa_mask)`.

### Semantic Shift: Interrupted Syscalls (`SA_RESTART`)
macOS `signal()` installs handlers with `SA_RESTART` by default, whereas `sigaction` with `sa_flags = 0` does not. When a signal is received:
* Interrupted blocking system calls (like `getch()`) will immediately return `EINTR` / `ERR` rather than resuming.
* This is beneficial for this project, as it allows the input loops in `play_one_run` and `death_screen` to immediately break and process the `sig_quit` shutdown flag.

### Code Changes

In [src/yeti.c](file:///Users/jonathan/yeti/src/yeti.c):

```diff
-    signal(SIGINT,  on_signal);
-    signal(SIGTERM, on_signal);
+    struct sigaction sa = { .sa_handler = on_signal };
+    sigemptyset(&sa.sa_mask);
+    sigaction(SIGINT,  &sa, NULL);
+    sigaction(SIGTERM, &sa, NULL);
```

Since `<signal.h>` and `_POSIX_C_SOURCE 200809L` are already defined at the top of the file, no extra headers or preprocessor flags are needed.

## Byte Budget Projection

* Current size: 10,105 bytes.
* Estimated change: +89 bytes (64-byte block replaced by 153-byte block).
* Projected size: 10,194 bytes (leaving 46 bytes of safety headroom under the 10,240-byte ceiling).
