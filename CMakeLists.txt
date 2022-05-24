cmake_minimum_required(VERSION 3.22)
project(ffmpegDemo C CXX)
set(CMAKE_CXX_STANDARD 14) 

add_subdirectory(lib/FFmpeg)
add_definitions(-DGL_SILENCE_DEPRECATION)

list(APPEND SOURCES src/main.cpp) 

add_executable(ffmpegDemo MACOSX_BUNDLE WIN32 ${SOURCES})

target_link_libraries(ffmpegDemo FFmpeg)
