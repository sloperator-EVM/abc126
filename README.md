# winrun

`winrun` is a Linux PE loader prototype for running selected Windows binaries by:

1. parsing a PE image,
2. mapping sections into memory,
3. applying base relocations,
4. resolving imports from a local WinAPI shim (`libwinapi.so`),
5. calling TLS callbacks,
6. transferring control to the PE entry point.

> Status: experimental. This is **not** a full Windows runtime.

## Quick start

```bash
./scripts/install.sh
WINRUN_DEBUG=1 ./build/winrun path/to/app.exe
```

## Arch Linux

```bash
./scripts/install-archlinux.sh
```

## NixOS / nix-shell

```bash
nix-shell
./scripts/install.sh
```

## WinAPI shim integration (`.wg`)

Place your WinAPI implementation in `.wg/*.c`, for example:

- `.wg/kernel32_console.c`
- `.wg/file.c`

Installer builds `lib/libwinapi.so` automatically.

Runtime lookup order:

1. `WINRUN_WINAPI_LIB` (explicit override)
2. `./lib/libwinapi.so`
3. `./.wg/libwinapi.so`
4. `./build/lib/libwinapi.so`

## What was tightened in this revision

- stricter PE header/size validation to reject malformed images early,
- safer mapped-memory RVA access helpers,
- relocation/import parsing bounds checks,
- TLS callback address handling with VA/RVA safety,
- better installer portability and deterministic build behavior.

## Import resolution behavior

- winrun is now intentionally bare-bones: it resolves imports by symbol name from your WinAPI shim (`libwinapi.so`) plus a small set of built-in CRT helpers needed for startup (`_initterm`, `_initterm_e`, `__acrt_iob_func`, `__stdio_common_vfprintf`, and runtime arg pointers).
- there is no fallback to process-global symbols (`RTLD_DEFAULT`). If a symbol is not in your `.wg` implementation (or the built-ins), loading fails fast with `unresolved import: DLL!Function`.

## Current limitations

- no delay-load import support,
- no import-by-ordinal handling,
- no full Win32 ABI/process initialization emulation,
- no SEH emulation (only Linux signal diagnostics).
