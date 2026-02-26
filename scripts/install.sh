#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
LIB_DIR="${ROOT_DIR}/lib"
WG_DIR="${ROOT_DIR}/.wg"
CC_BIN="${CC:-cc}"

log() {
  printf '[install] %s\n' "$*"
}

require_tool() {
  if ! command -v "$1" >/dev/null 2>&1; then
    log "missing required tool: $1"
    exit 1
  fi
}

jobs() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  else
    getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1
  fi
}

log "checking required tools"
require_tool cmake
require_tool "${CC_BIN}"

log "configuring and building winrun"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j"$(jobs)"

mkdir -p "${LIB_DIR}"

if compgen -G "${WG_DIR}/*.c" >/dev/null; then
  log "building lib/libwinapi.so from .wg sources"
  "${CC_BIN}" -shared -fPIC "${WG_DIR}"/*.c -o "${LIB_DIR}/libwinapi.so"
  log "built ${LIB_DIR}/libwinapi.so"
elif [[ -f "${BUILD_DIR}/lib/libwinapi.so" ]]; then
  log "copying CMake-built libwinapi.so from build/lib"
  cp -f "${BUILD_DIR}/lib/libwinapi.so" "${LIB_DIR}/libwinapi.so"
  log "copied ${LIB_DIR}/libwinapi.so"
else
  log "no .wg/*.c found, skipping WinAPI shim build"
  log "add your WinAPI implementation files under .wg/"
fi

log "done"
log "run: WINRUN_DEBUG=1 ${BUILD_DIR}/winrun path/to/program.exe"
