cmake_minimum_required(VERSION 3.19)
project(vulkan_engine VERSION 1.0)

add_executable(${PROJECT_NAME})
target_compile_features(vulkan_engine PUBLIC cxx_std_20)

add_subdirectory(source)
add_subdirectory(external)
