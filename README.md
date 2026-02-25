# winrun

`winrun` is a Linux-hosted prototype PE loader intended to execute Windows binaries by mapping their image and binding imports to a Linux WinAPI compatibility layer (`libwinapi.so`).

## Current capabilities

- PE parser for DOS + NT headers, section table, data directories, and RVA translation.
- Memory mapper that tries preferred image base first, falls back to arbitrary base, copies headers/sections, zero-initializes BSS, and applies per-section memory protections.
- Relocation processor for common base relocation types:
  - `IMAGE_REL_BASED_DIR64` (PE32+)
  - `IMAGE_REL_BASED_HIGHLOW` (PE32)
- Import resolver that walks import descriptors and patches IAT entries using symbols from a WinAPI shim library.
- TLS callback initialization (`DLL_PROCESS_ATTACH`).
- Signal handling for crash diagnostics (`SIGSEGV`, `SIGILL`, `SIGBUS`, `SIGABRT`) with optional backtraces (`WINRUN_DEBUG=1`).

## Quick bootstrap (recommended)

```bash
./scripts/install.sh
```

This builds `build/winrun` and, when `.wg/*.c` files exist, compiles them into `lib/libwinapi.so`.

## Build manually

```bash
cmake -S . -B build
cmake --build build
```

If `.wg/*.c` exists, CMake also builds `build/lib/libwinapi.so`.

## Run

```bash
WINRUN_DEBUG=1 ./build/winrun path/to/program.exe [args...]
```

WinAPI shim lookup order:

1. `WINRUN_WINAPI_LIB` (if set)
2. `./lib/libwinapi.so`
3. `./.wg/libwinapi.so`
4. `./build/lib/libwinapi.so`

## NixOS support

A `shell.nix` is provided. Use:

```bash
nix-shell
./scripts/install.sh
```

See full guide: [`docs/USAGE.md`](docs/USAGE.md)

## `.wg` integration

Place your WinAPI compatibility function sources in `.wg/*.c` and re-run installer.

See `.wg/README.md` for details.

## Notes / limitations

- This is an early runtime prototype, not full Windows emulation.
- Entry-point execution currently does a direct function jump and does not yet provide complete Win32 process startup semantics/ABI thunking.
- Import-by-ordinal and delay-loaded imports are not yet implemented.
- SEH translation is not yet complete; signal hooks currently provide debugging aid only.

## Roadmap alignment

The codebase is structured so each major source file maps to one of the planned subsystems:

- `src/pe_parser.c` → PE file parser
- `src/memory_mapper.c` → memory mapper
- `src/relocations.c` → relocation processor
- `src/import_resolver.c` → import resolver + API binding
- `src/runtime.c` → TLS + entry-point handler
- `src/signals.c` → crash/signal integration

This gives a practical base for iteratively adding calling-convention thunks, handle tables, richer DLL loading, and structured exception emulation.
