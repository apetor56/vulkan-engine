cmake_minimum_required(VERSION 3.19)
project(VulkanEngine VERSION 0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(source)
add_subdirectory(external)

if (CMAKE_GENERATOR MATCHES "Visual Studio")   
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()
