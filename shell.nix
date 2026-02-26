{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = with pkgs; [
    cmake
    gcc
    gnumake
    pkg-config
    wayland
    wayland-protocols
    gdb
    strace
    binutils
    patchelf
  ];

  shellHook = ''
    export WINRUN_DEBUG=''${WINRUN_DEBUG:-1}
    echo "winrun dev shell ready"
    echo "bootstrap: ./scripts/install.sh"
    echo "run:       ./build/winrun path/to/program.exe"
  '';
}
