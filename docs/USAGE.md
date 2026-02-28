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
