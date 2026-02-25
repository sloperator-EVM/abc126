# winrun setup and usage guide

## 1) Standard Linux bootstrap

```bash
./scripts/install.sh
```

This will:
- configure + build `winrun` into `build/winrun`
- compile `.wg/*.c` into `lib/libwinapi.so` when `.wg` sources exist

## 2) NixOS / nix-shell workflow

### Enter the shell

```bash
nix-shell
```

### Build everything

```bash
./scripts/install.sh
```

### Run

```bash
WINRUN_DEBUG=1 ./build/winrun path/to/program.exe [args...]
```

## 3) Integrating your existing `.wg` WinAPI functions

1. Place all WinAPI source files under `.wg/`.
2. Ensure exported symbols match Windows import names.
3. Re-run installer:

```bash
./scripts/install.sh
```

4. Run your PE:

```bash
./build/winrun your_program.exe
```

If your shim lives elsewhere, point winrun at it:

```bash
WINRUN_WINAPI_LIB=/absolute/path/to/libwinapi.so ./build/winrun your_program.exe
```
