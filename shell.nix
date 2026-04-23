{pkgs ? import <nixpkgs> {}}:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    git
    gh

    ftxui
    gh-token

    cmake
    ninja
    ccache
    clang-tools

    vscode-json-languageserver
  ];

  CMAKE_CXX_COMPILER_LAUNCHER = "ccache";
}
