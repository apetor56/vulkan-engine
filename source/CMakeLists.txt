set(SOURCES
    main.cpp Config.hpp
    Application.cpp Application.hpp
    VulkanInstance.cpp VulkanInstance.hpp
    DebugMessenger.cpp DebugMessenger.hpp
    Window.cpp Window.hpp 
    PhysicalDevice.cpp PhysicalDevice.hpp
    QueueFamilyIndices.cpp QueueFamilyIndices.hpp
    LogicalDevice.cpp LogicalDevice.hpp
    Swapchain.cpp Swapchain.hpp
    Pipeline.cpp Pipeline.hpp
    Shader.cpp Shader.hpp
    CommandBuffer.cpp CommandBuffer.hpp  
)

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})
