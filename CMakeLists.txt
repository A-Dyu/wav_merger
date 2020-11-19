cmake_minimum_required(VERSION 3.15)
project(wav_merger)

set(CMAKE_CXX_STANDARD 17)

add_executable(wav_merger main.cpp wav_file.cpp wav_file.h)