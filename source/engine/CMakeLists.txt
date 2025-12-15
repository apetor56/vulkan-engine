add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /permissive-
        /W4
        /w14640
	)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -pedantic
	)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SHADER_BINARIES_DIR = "${CMAKE_BINARY_DIR}/shaders"
    ASSETS_DIR          = "${CMAKE_SOURCE_DIR}/assets"
)

target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    gachix.cpp gachix.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/core"
)
add_dependencies(${PROJECT_NAME} shaders)
