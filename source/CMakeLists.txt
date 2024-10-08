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
    TEXTURE_DIR="${CMAKE_SOURCE_DIR}/source/textures"  
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
    core/Pipeline.hpp              core/Pipeline.cpp
    core/ShaderModule.hpp          core/ShaderModule.cpp
    core/Vertex.hpp
    core/Buffer.hpp
    core/MemoryAllocator.hpp       core/MemoryAllocator.cpp
    core/Image.hpp                 core/Image.cpp
)

set(COMMAND
    core/command/CommandPool.hpp
    core/command/BaseCommandBuffer.hpp            core/command/BaseCommandBuffer.cpp
    core/command/GraphicsCommandBuffer.hpp        core/command/GraphicsCommandBuffer.cpp
    core/command/TransferCommandBuffer.hpp        core/command/TransferCommandBuffer.cpp
)

set(DESCRIPTOR
    core/descriptor/DescriptorSetLayout.hpp        core/descriptor/DescriptorSetLayout.cpp
    core/descriptor/DescriptorPool.hpp             core/descriptor/DescriptorPool.cpp
)

set(SHADER_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple.frag"
)

source_group("Command" FILES ${COMMAND})
source_group("Descriptor" FILES ${DESCRIPTOR})

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
    "${CMAKE_CURRENT_SOURCE_DIR}/core"
)
add_dependencies(${PROJECT_NAME} Shaders)
