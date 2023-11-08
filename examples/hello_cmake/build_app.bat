@echo off

rmdir /s /q build
cmake -S . -B build -G "Ninja" -D CMAKE_TOOLCHAIN_FILE=%IDF_PATH%/tools/cmake/toolchain-esp32.cmake -DTARGET=esp32
cmake --build build