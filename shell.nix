{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    ftxui
    gh-token

    cmake
    ninja
    ccache

    clang-tools
  ];

  CMAKE_CXX_COMPILER_LAUNCHER = "ccache";
}
