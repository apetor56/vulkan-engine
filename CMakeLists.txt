cmake_minimum_required(VERSION 3.19)
project(VulkanEngine VERSION 1.0)

add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

add_subdirectory(source)
add_subdirectory(external)

if (CMAKE_GENERATOR MATCHES "Visual Studio")   
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()
