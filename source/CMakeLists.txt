add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

set(CORE
    core/Config.hpp
    core/VulkanInstance.cpp core/VulkanInstance.hpp
    core/DebugMessenger.cpp core/DebugMessenger.hpp
    core/Window.cpp core/Window.hpp 
    core/PhysicalDevice.cpp core/PhysicalDevice.hpp
    core/QueueFamilyIndices.cpp core/QueueFamilyIndices.hpp
    core/LogicalDevice.cpp core/LogicalDevice.hpp
    core/Swapchain.cpp core/Swapchain.hpp
    core/Pipeline.cpp core/Pipeline.hpp
    core/Shader.cpp core/Shader.hpp
    core/CommandBuffer.cpp core/CommandBuffer.hpp  
)

source_group("Core" FILES ${CORE})

target_sources(${PROJECT_NAME} PRIVATE
	main.cpp
	Application.cpp Application.hpp
	${CORE}
)

