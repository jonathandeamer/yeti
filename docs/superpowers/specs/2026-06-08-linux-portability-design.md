# yeti — Linux Portability Design

Restore robust Unix/Linux portability to `yeti` (specifically targeting Raspbian 11 Bullseye on armv6l) while maintaining the 10,240-byte source budget constraint.

## Goal

Achieve reliable signal handling and clean compile/run behavior on Linux systems without platform-conditional `#ifdef` compilation flags.

## Approach: POSIX `sigaction`

Standard C99 `signal()` has undefined persistence semantics on POSIX platforms, meaning the handler can be reset to `SIG_DFL` after the first trigger. On macOS (BSD), it remains persistent by default. 

To ensure persistent signal handling on Linux (glibc/musl) and macOS, we will use POSIX `sigaction`.

### Code Changes

In [src/yeti.c](file:///Users/jonathan/yeti/src/yeti.c):

```diff
-    signal(SIGINT,  on_signal);
-    signal(SIGTERM, on_signal);
+    struct sigaction sa = { .sa_handler = on_signal };
+    sigaction(SIGINT,  &sa, NULL);
+    sigaction(SIGTERM, &sa, NULL);
```

Since `<signal.h>` and `_POSIX_C_SOURCE 200809L` are already defined at the top of the file, no extra headers or preprocessor flags are needed.

## Byte Budget Projection

* Current size: 10,105 bytes.
* Estimated change: +58 bytes.
* Projected size: 10,163 bytes (under the 10,240-byte ceiling).
