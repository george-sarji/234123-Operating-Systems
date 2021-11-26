cmake_minimum_required(VERSION 3.15)
project(smash)

set(CMAKE_CXX_STANDARD 11)

add_executable(smash  BuiltIn.h BuiltIn.cpp Command.h Commands.cpp Commands.h External.h External.cpp JobsList.h JobsList.cpp
        Pipe.h Shell.h Shell.cpp signals.h signals.cpp smash.cpp Special.cpp Utility.h Utility.cpp)