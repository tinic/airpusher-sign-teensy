#!/bin/sh

export PATH="/opt/gcc-arm-none-eabi-10-2020-q4-major/bin:${PATH}"
set -e
build_type="Ninja"

rm -rf build_debug
mkdir -p build_debug
cd build_debug
cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug ..
ninja
cd ..

rm -rf build_release
mkdir -p build_release
cd build_release
cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..
ninja
cd ..
