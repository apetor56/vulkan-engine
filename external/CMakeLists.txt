include(FetchContent)

find_package(Vulkan REQUIRED COMPONENTS glslc)

option(GLFW_BUILD_DOCS "Build GLFW documentation" OFF)
option(GLFW_BUILD_TESTS "Build GLFW tests" OFF)
option(GLFW_BUILD_EXAMPLES "Build GLFW examples" OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "" OFF)
option(ASSIMP_BUILD_OBJ_IMPORTER "Enable OBJ importer" ON)
option(ASSIMP_BUILD_GLTF_IMPORTER "Enable GLTF importer" ON)
option(ASSIMP_NO_EXPORT "Disable assimp exporter" ON)
option(ASSIMP_BUILD_SAMPLES "Build Assimp sample applications" OFF)
option(ASSIMP_BUILD_TESTS "Build Assimp unit tests" OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS "Build Assimp tools" OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.3
)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(stb)
FetchContent_MakeAvailable(assimp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${Vulkan_INCLUDE_DIR}
    ${glfw_SOURCE_DIR}/include
    ${spdlog_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/_deps/stb-src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
    assimp
)
