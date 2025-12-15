include(ShaderCompilation)

set(SHADER_SOURCE
    "Simple.vert"
    "Simple.frag"
    "Mesh.vert"
    "Mesh.frag"
    "Skybox.vert"
    "Skybox.frag"
)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
set(COMPILED_SHADERS_OUTPUT)
compile_shaders("${SHADER_SOURCE}" COMPILED_SHADERS_OUTPUT)
add_custom_target(shaders DEPENDS "${COMPILED_SHADERS_OUTPUT}")
target_sources(shaders PRIVATE "${SHADER_SOURCE}")

add_dependencies(${PROJECT_NAME} shaders)
