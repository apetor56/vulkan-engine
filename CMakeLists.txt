cmake_minimum_required(VERSION 3.14)
project(Vulkan VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -extra -pedantic -g)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

set(SOURCE ${CMAKE_SOURCE_DIR}/source)

add_executable(${PROJECT_NAME}
    ${SOURCE}/main.cpp
    ${SOURCE}/window.cpp
    ${SOURCE}/application.cpp
    ${SOURCE}/pipeline.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARY}
    glfw
)