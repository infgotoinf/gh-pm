{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    ftxui

    cmake
    ninja
    ccache

    clang-tools
  ];

  CMAKE_CXX_COMPILER_LAUNCHER = "ccache";
}
