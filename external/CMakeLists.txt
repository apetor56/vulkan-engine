include(FetchContent)

find_package(Vulkan REQUIRED COMPONENTS glslc)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

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
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_Declare(
    GPUOpen
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.1.0
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
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(GPUOpen)
FetchContent_MakeAvailable(stb)
FetchContent_MakeAvailable(assimp)

target_include_directories(${PROJECT_NAME} PRIVATE
    Vulkan::Headers
    ${glfw_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/_deps/stb-src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
	spdlog::spdlog
    glm::glm
    GPUOpen::VulkanMemoryAllocator
    assimp
)
