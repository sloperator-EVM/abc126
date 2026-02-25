# .wg WinAPI shim sources

Put your WinAPI compatibility functions in this directory as C source files (`*.c`).

Examples:
- `.wg/kernel32_console.c`
- `.wg/file_io.c`

`./scripts/install.sh` will automatically compile all `.wg/*.c` into:

- `lib/libwinapi.so`

`winrun` will attempt to load WinAPI implementations in this order:

1. `WINRUN_WINAPI_LIB` (if set)
2. `./lib/libwinapi.so`
3. `./.wg/libwinapi.so`
4. `./build/lib/libwinapi.so`

Exported symbol names must match PE import names (e.g. `WriteFile`, `GetStdHandle`, `ExitProcess`).
