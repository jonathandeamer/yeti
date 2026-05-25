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

## Controls

| Key | Action |
| --- | --- |
| Left / Right | lean |
| Q, ESC | quit |
| R | restart from death |

## License

Dual-licensed under your choice of MIT or CC0-1.0. See `LICENSES/MIT.txt` and `LICENSES/CC0-1.0.txt`.

## Independent reimplementation

`yeti` is implemented without source contact with any predecessor or sibling ski game. See `FORBIDDEN-SOURCES.md` for the discipline that binds every contributor.
