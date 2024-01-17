@echo off

rmdir /s /q build
cmake -S . -B build -G "Ninja" -D CMAKE_TOOLCHAIN_FILE=%IDF_PATH%/tools/cmake/toolchain-%1.cmake -DTARGET=%1
cmake --build build