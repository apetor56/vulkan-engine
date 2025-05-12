include(ShaderCompilation)

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
    SHADER_BINARIES_DIR="${CMAKE_BINARY_DIR}/shaders"
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

set(CORE
    core/Config.hpp             
    core/Engine.hpp                core/Engine.cpp
    core/VulkanInstance.hpp        core/VulkanInstance.cpp
    core/DebugMessenger.hpp        core/DebugMessenger.cpp
    core/Window.hpp                core/Window.cpp 
    core/PhysicalDevice.hpp        core/PhysicalDevice.cpp
    core/QueueFamilyIDs.hpp        core/QueueFamilyIDs.cpp
    core/LogicalDevice.hpp         core/LogicalDevice.cpp
    core/Swapchain.hpp             core/Swapchain.cpp
    core/RenderPass.hpp            core/RenderPass.cpp
    core/Framebuffer.hpp           core/Framebuffer.cpp
    core/Pipeline.hpp              core/Pipeline.cpp
    core/ShaderModule.hpp          core/ShaderModule.cpp
    core/Vertex.hpp
    core/Buffer.hpp
    core/MemoryAllocator.hpp       core/MemoryAllocator.cpp
    core/Image.hpp                 core/Image.cpp
    core/Frame.hpp                 core/Frame.cpp
    core/SyncObjects.hpp           core/SyncObjects.cpp
    core/Constants.hpp
    core/Loader.hpp                core/Loader.cpp
    core/Node.hpp                  core/Node.cpp
    core/Material.hpp              core/Material.cpp
    core/Mesh.hpp
    core/Camera.hpp                core/Camera.cpp
)

set(UTILS
    utils/NonCopyable.hpp
    utils/NonMovable.hpp
    utils/Common.hpp
)

set(COMMAND
    core/command/CommandPool.hpp
    core/command/BaseCommandBuffer.hpp            core/command/BaseCommandBuffer.cpp
    core/command/GraphicsCommandBuffer.hpp        core/command/GraphicsCommandBuffer.cpp
    core/command/TransferCommandBuffer.hpp        core/command/TransferCommandBuffer.cpp
)

set(DESCRIPTOR
    core/descriptor/DescriptorSetLayout.hpp        core/descriptor/DescriptorSetLayout.cpp
    core/descriptor/DescriptorAllocator.hpp        core/descriptor/DescriptorAllocator.cpp
    core/descriptor/DescriptorWriter.hpp           core/descriptor/DescriptorWriter.cpp
)

set(SHADER_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Simple.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Simple.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Mesh.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Mesh.frag"
#    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Structures.glsl"
)

source_group("Command" FILES ${COMMAND})
source_group("Descriptor" FILES ${DESCRIPTOR})
source_group("Utilities" FILES ${UTILS})

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
set(COMPILED_SHADERS_OUTPUT)
compile_shaders("${SHADER_SOURCE}" COMPILED_SHADERS_OUTPUT)
add_custom_target(Shaders DEPENDS "${COMPILED_SHADERS_OUTPUT}")
target_sources(Shaders PRIVATE "${SHADER_SOURCE}")

source_group("Core" FILES ${CORE})
target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    Application.cpp Application.hpp
    "${CORE}"
    "${COMMAND}"
    "${DESCRIPTOR}"
)
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/core"
)
add_dependencies(${PROJECT_NAME} Shaders)
