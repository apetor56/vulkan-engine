@echo off

if exist build (
	cmake --build build
	exit
)

echo Pass the number to select your toolset:
echo    Generator                Compiler
echo 1) Visual Studio 17 2022    msvc143,
echo 2) Visual Studio 17 2022    clang-cl
echo 3) MinGW Makefile           clang++
echo 4) MinGW Makefile           g++

set /p userInput=

if      %userInput% == 1 ( cmake -S . --preset visual-studio-msvc-debug ) ^
else if %userInput% == 2 ( cmake -S . --preset visual-studio-clangcl-debug ) ^
else if %userInput% == 3 ( cmake -S . --preset mingw-makefiles-clang-debug ) ^
else if %userInput% == 4 ( cmake -S . --preset mingw-makefiles-gcc-debug )
