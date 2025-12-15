add_library(vulkan_renderer)
target_compile_features(vulkan_renderer PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(vulkan_renderer PRIVATE
        /permissive-
        /W4
        /w14640
	)
else()
    target_compile_options(vulkan_renderer PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -pedantic
	)
endif()

target_compile_definitions(vulkan_renderer PRIVATE 
    SHADER_BINARIES_DIR="${CMAKE_BINARY_DIR}/shaders"
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

set(SOURCE
    config.hpp
    renderer.hpp              renderer.cpp
    vulkan_instance.hpp       vulkan_instance.cpp
    debug_messenger.hpp       debug_messenger.cpp
    window.hpp                window.cpp
    queue_family_ids.hpp      queue_family_ids.cpp
    swapchain.hpp             swapchain.cpp
    shader_module.hpp         shader_module.cpp
    vertex.hpp
    buffer.hpp
    memory_allocator.hpp      memory_allocator.cpp
    frame.hpp                 frame.cpp
    constants.hpp
    node.hpp                  node.cpp
    material.hpp              material.cpp
    mesh.hpp
    camera.hpp                camera.cpp
)

set(DEVICE
    device/physical_device.hpp      device/physical_device.cpp
    device/logical_device.hpp       device/logical_device.cpp
)

set(PIPELINE
    pipeline/pipeline.hpp       pipeline/pipeline.cpp
)

set(TEXTURE
    texture/image.hpp       texture/image.cpp
    texture/sampler.hpp     texture/sampler.cpp
)

set(COMMAND
    command/command_pool.hpp
    command/base_command_buffer.hpp          command/base_command_buffer.cpp
    command/graphics_command_buffer.hpp      command/graphics_command_buffer.cpp
    command/transfer_command_buffer.hpp      command/transfer_command_buffer.cpp
)

set(DESCRIPTOR
    descriptor/descriptor_set_layout.hpp      descriptor/descriptor_set_layout.cpp
    descriptor/descriptor_allocator.hpp       descriptor/descriptor_allocator.cpp
    descriptor/descriptor_writer.hpp          descriptor/descriptor_writer.cpp
)

set(SYNCHRONIZATION
    synchronization/sync_objects.hpp        synchronization/sync_objects.cpp
)

source_group("device" FILES ${DEVICE})
source_group("pipeline" FILES ${PIPELINE})
source_group("texture" FILES ${TEXTURE})
source_group("command" FILES ${COMMAND})
source_group("descriptor" FILES ${DESCRIPTOR})
source_group("synchronization" FILES ${DESCRIPTOR})

target_sources(vulkan_renderer PRIVATE
    "${SOURCE}"
    "${DEVICE}"
    "${PIPELINE}"
    "${TEXTURE}"
    "${COMMAND}"
    "${DESCRIPTOR}"
    "${SYNCHRONIZATION}"
)
