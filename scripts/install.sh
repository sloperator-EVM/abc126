#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
LIB_DIR="${ROOT_DIR}/lib"
WG_DIR="${ROOT_DIR}/.wg"
WG_TMP_DIR="${WG_DIR}/tmp"
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

install_archlinux_deps_if_available() {
  if [[ -f /etc/arch-release ]] && command -v pacman >/dev/null 2>&1; then
    log "detected Arch Linux; installing build dependencies via pacman"
    local pacman_cmd=(pacman)
    if [[ "${EUID}" -ne 0 ]]; then
      require_tool sudo
      pacman_cmd=(sudo pacman)
    fi
    "${pacman_cmd[@]}" -Sy --needed --noconfirm \
      base-devel \
      cmake \
      gcc \
      pkgconf \
      wayland \
      wayland-protocols \
      libinput
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
install_archlinux_deps_if_available
require_tool cmake
require_tool "${CC_BIN}"

log "configuring and building winrun"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j"$(jobs)"

mkdir -p "${LIB_DIR}"

if compgen -G "${WG_DIR}/*.c" >/dev/null; then
  require_tool pkg-config

  if ! pkg-config --exists wayland-client libinput libudev; then
    log "wayland-client, libinput, and libudev development files are required to build libwinapi.so"
    log "install packages providing wayland-client.pc, libinput.pc, and libudev.pc and rerun"
    exit 1
  fi

  log "building lib/libwinapi.so from .wg sources"
  WG_CFLAGS="$(pkg-config --cflags wayland-client libinput libudev)"
  WG_LIBS="$(pkg-config --libs wayland-client libinput libudev)"
  "${CC_BIN}" -shared -fPIC -Wno-unused-parameter -Wno-sign-compare \
    -I"${WG_DIR}" -I"${WG_TMP_DIR}" \
    ${WG_CFLAGS} \
    "${WG_DIR}"/*.c \
    "${WG_TMP_DIR}/single-pixel-buffer-v1.c" \
    "${WG_TMP_DIR}/viewporter.c" \
    "${WG_TMP_DIR}/wlr-layer-shell-unstable-v1.c" \
    "${WG_TMP_DIR}/wlr-virtual-pointer-unstable-v1.c" \
    "${WG_TMP_DIR}/xdg-shell.c" \
    ${WG_LIBS} -pthread \
    -o "${LIB_DIR}/libwinapi.so"
  log "built ${LIB_DIR}/libwinapi.so"

elif [[ -f "${BUILD_DIR}/lib/libwinapi.so" ]]; then
  log "copying CMake-built libwinapi.so from build/lib"
  cp -f "${BUILD_DIR}/lib/libwinapi.so" "${LIB_DIR}/libwinapi.so"
  log "copied ${LIB_DIR}/libwinapi.so"
else
  log "no .wg/*.c found, skipping WinAPI shim build"
  log "add your WinAPI implementation files under .wg/"
fi

if [[ -f "${LIB_DIR}/libwinapi.so" ]]; then
  log "checking libwinapi.so for unresolved symbols"
  ldd -r "${LIB_DIR}/libwinapi.so" >/dev/null
  log "libwinapi.so symbol check passed"
fi

if [[ -f "${ROOT_DIR}/.wg/test_win.exe" ]]; then
  log "running loader smoke test against .wg/test_win.exe"
  set +e
  smoke_out="$(WINRUN_DEBUG=1 WINRUN_WINAPI_LIB="${LIB_DIR}/libwinapi.so" "${BUILD_DIR}/winrun" "${ROOT_DIR}/.wg/test_win.exe" 2>&1)"
  smoke_rc=$?
  set -e
  printf '%s\n' "${smoke_out}" | sed 's/^/[smoke] /'
  if printf '%s' "${smoke_out}" | grep -q "caught signal"; then
    log "loader smoke test crashed"
    exit 1
  fi
  if printf '%s' "${smoke_out}" | grep -q "unresolved import:"; then
    log "loader smoke test has unresolved imports"
    exit 1
  fi
  if [[ ${smoke_rc} -ne 0 ]]; then
    log "loader smoke test failed (rc=${smoke_rc})"
    exit 1
  fi
  log "loader smoke test completed (rc=${smoke_rc})"
fi

log "done"
log "run: WINRUN_DEBUG=1 ${BUILD_DIR}/winrun path/to/program.exe"
