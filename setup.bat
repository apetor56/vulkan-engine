@echo off

glslc ./shaders/simple.vert -o ./shaders/simple.vert.spv
glslc ./shaders/simple.frag -o ./shaders/simple.frag.spv

if not exist build (
    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE="Debug"
)
cmake --build build
