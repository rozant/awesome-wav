cmake_minimum_required (VERSION 2.6)
Project (awesome-wav)

#version number
set (awesome-wav_VERSION_MAJOR 1)
set (awesome-wav_VERSION_MINOR 0)
set (awesome-wav_VERSION_SUB_MINOR 0)

#flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -lz -D_FILE_OFFSET_BITS=64")

add_subdirectory(src)
