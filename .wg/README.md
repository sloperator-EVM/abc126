# .wg WinAPI shim sources

Add your WinAPI compatibility sources here as `.c` files.

Example exports (names must match PE imports):
- `GetStdHandle`
- `WriteFile`
- `ExitProcess`

Build using:

```bash
./scripts/install.sh
```

This will create `lib/libwinapi.so` from all `.wg/*.c` files.
