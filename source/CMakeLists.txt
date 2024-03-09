set(SOURCES
    main.cpp config.hpp
    application.cpp application.hpp
    vulkan_instance.cpp vulkan_instance.hpp
    debug_messenger.cpp debug_messenger.hpp
    window.cpp window.hpp 
    physical_device.cpp physical_device.hpp
    queue_family_indices.cpp queue_family_indices.hpp
    logical_device.cpp logical_device.hpp
    swapchain.cpp swapchain.hpp
    pipeline.cpp pipeline.hpp
    shader.cpp shader.hpp
    command_buffer.cpp command_buffer.hpp  
)

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})
