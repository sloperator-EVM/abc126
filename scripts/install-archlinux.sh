#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

log() {
  printf '[arch-install] %s\n' "$*"
}

require_tool() {
  if ! command -v "$1" >/dev/null 2>&1; then
    log "missing required tool: $1"
    exit 1
  fi
}

require_tool git
require_tool pacman

if [[ "${EUID}" -eq 0 ]]; then
  PACMAN_CMD=(pacman)
else
  require_tool sudo
  PACMAN_CMD=(sudo pacman)
fi

log "installing build dependencies via pacman"
"${PACMAN_CMD[@]}" -Sy --needed --noconfirm \
  base-devel \
  cmake \
  gcc \
  pkgconf \
  wayland \
  wayland-protocols

log "running project installer"
"${ROOT_DIR}/scripts/install.sh"

log "done"
