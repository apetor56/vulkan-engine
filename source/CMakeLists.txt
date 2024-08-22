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

target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_BINARIES_DIR="${CMAKE_BINARY_DIR}/shaders")

set(CORE
    core/Config.hpp             
    core/VulkanInstance.cpp        core/VulkanInstance.hpp
    core/DebugMessenger.cpp        core/DebugMessenger.hpp
    core/Window.cpp                core/Window.hpp 
    core/PhysicalDevice.cpp        core/PhysicalDevice.hpp
    core/QueueFamilyIndices.cpp    core/QueueFamilyIndices.hpp
    core/LogicalDevice.cpp         core/LogicalDevice.hpp
    core/Swapchain.cpp             core/Swapchain.hpp
    core/Pipeline.cpp              core/Pipeline.hpp
    core/ShaderModule.cpp          core/ShaderModule.hpp
    core/CommandBuffer.cpp         core/CommandBuffer.hpp  
)

set(SHADER_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple.frag"
)

set(COMPILED_SHADERS_OUTPUT)
compile_shaders("${SHADER_SOURCE}" COMPILED_SHADERS_OUTPUT)
add_custom_target(Shaders DEPENDS "${COMPILED_SHADERS_OUTPUT}")
target_sources(Shaders PRIVATE "${SHADER_SOURCE}")

source_group("Core" FILES ${CORE})
target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    Application.cpp Application.hpp
    "${CORE}"
)
add_dependencies(${PROJECT_NAME} Shaders)
