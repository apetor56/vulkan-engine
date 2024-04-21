@echo off

echo Pass the number to select your toolset:
echo 1) Visual Studio 17 2022: mvsc143,
echo 2) Visual Studio 17 2022: clang-cl

set /p userInput=

if 		%userInput% == 1 ( cmake -S . --preset vs2022-mvsc-debug ) ^
else if %userInput% == 2 ( cmake -S . --preset vs2022-clangcl-debug ) ^
