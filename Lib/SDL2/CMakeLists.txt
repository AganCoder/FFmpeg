cmake_minimum_required(VERSION 3.22)
project(SDL2)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2  REQUIRED IMPORTED_TARGET sdl2)

add_library(SDL2 INTERFACE IMPORTED GLOBAL)

target_link_libraries(FFmpeg INTERFACE
    PkgConfig::SDL2
)