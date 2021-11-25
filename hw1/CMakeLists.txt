cmake_minimum_required(VERSION 3.15)
project(smash)

set(CMAKE_CXX_STANDARD 11)

add_executable(smash  signals.cpp signals.h Commands.cpp Commands.h smash.cpp)