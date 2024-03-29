@echo off

echo Pass the number to select your toolset:
echo 1) Visual Studio 19 2022: mvsc143,
echo 2) MinGW:                 g++,

set /p userInput=

if %userInput% == 1 (
    cmake -S . --preset vs2022-mvsc-debug
    exit
)
if %userInput% == 2 (
    cmake -S . -DCMAKE_CXX_COMPILER=g++ --preset mingw-g++-debug
    exit
)
