cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_GENERATOR Ninja)

project(PM)


# This chunk of code exists to make my clangd LSP in Helix work with SDL3
# https://mzhang.io/posts/2022-03-03-clangd-in-nix/
# Generate the `compile_commands.json` file.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()


find_package(ftxui REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(game)

target_sources(game
  PRIVATE
    main/main.cpp

  PUBLIC
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS
      main/include
    FILES
)

target_link_libraries(game
  PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
    nlohmann_json::nlohmann_json
)
