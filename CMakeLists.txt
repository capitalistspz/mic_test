cmake_minimum_required(VERSION 4.1)
project(mic_test)

set(CMAKE_CXX_STANDARD 23)

add_executable(mic_test main.cpp
        wav.hpp file.hpp ints.hpp bstype.hpp
        mic.hpp)

wut_create_rpx(mic_test)