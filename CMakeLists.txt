cmake_minimum_required(VERSION 3.15)
project(wav_merger)

set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE( Boost 1.74 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(wav_merger main.cpp wav_file.cpp wav_file.h)
target_link_libraries(wav_merger ${Boost_PROGRAM_OPTIONS_LIBRARY})