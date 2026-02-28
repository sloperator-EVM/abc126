# Usage

## Build

```bash
./scripts/install.sh
```

## Run

```bash
WINRUN_DEBUG=1 ./build/winrun path/to/program.exe [args...]
```

## Optional: custom WinAPI shim path

```bash
WINRUN_WINAPI_LIB=/absolute/path/to/libwinapi.so ./build/winrun path/to/program.exe
```

## NixOS

```bash
nix-shell
./scripts/install.sh
```


## Smoke test semantics

`./scripts/install.sh` now treats any smoke-test `unresolved import:` line or non-zero smoke exit code as a hard failure.
This is intentional: a successful build must also pass import resolution for the bundled sample PE.


## Optional: strict section protections

By default, `winrun` keeps mapped PE pages RWX-compatible during early runtime to avoid crashes in some CRT startup paths.
Set `WINRUN_STRICT_PROT=1` to force section-flag-based `mprotect` before entering PE code.

```bash
WINRUN_STRICT_PROT=1 WINRUN_DEBUG=1 ./build/winrun path/to/program.exe
```
