#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
LIB_DIR="${ROOT_DIR}/lib"
WG_DIR="${ROOT_DIR}/.wg"

log() {
  printf '[install] %s\n' "$*"
}

require_tool() {
  if ! command -v "$1" >/dev/null 2>&1; then
    log "missing required tool: $1"
    exit 1
  fi
}

log "checking required tools"
require_tool cmake
require_tool gcc

log "configuring and building winrun"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

mkdir -p "${LIB_DIR}"

if compgen -G "${WG_DIR}/*.c" > /dev/null; then
  log "detected .wg C sources, building lib/libwinapi.so"
  gcc -shared -fPIC "${WG_DIR}"/*.c -o "${LIB_DIR}/libwinapi.so"
  log "built ${LIB_DIR}/libwinapi.so"
elif [[ -f "${BUILD_DIR}/lib/libwinapi.so" ]]; then
  log "copying CMake-built libwinapi.so from build/lib"
  cp -f "${BUILD_DIR}/lib/libwinapi.so" "${LIB_DIR}/libwinapi.so"
  log "copied ${LIB_DIR}/libwinapi.so"
else
  log "no .wg/*.c found; skipping WinAPI shim build"
  log "add your WinAPI source files under .wg/ or place a library at lib/libwinapi.so"
fi

log "done"
log "run with: WINRUN_DEBUG=1 ${BUILD_DIR}/winrun path/to/program.exe"
