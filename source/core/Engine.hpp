#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "MemoryAllocator.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"

#include "command/CommandPool.hpp"
#include "command/GraphicsCommandBuffer.hpp"

#include "descriptor/DescriptorSetLayout.hpp"
#include "descriptor/DescriptorPool.hpp"

namespace ve {

class Engine {
public:
    Engine();
    ~Engine();

    void run();
    void render();

private:
    ve::VulkanInstance m_vulkanInstance{};
    ve::Window m_window;
    ve::PhysicalDevice m_physicalDevice;
    ve::LogicalDevice m_logicalDevice;
    ve::MemoryAllocator m_memoryAllocator;
    ve::Swapchain m_swapchain;
    std::optional< ve::Pipeline > m_pipeline;

    ve::CommandPool< ve::GraphicsCommandBuffer > m_graphicsCommandPool;
    std::vector< ve::GraphicsCommandBuffer > m_commandBuffers;

    vk::Fence m_immediateSubmitFence{};
    ve::CommandPool< ve::TransferCommandBuffer > m_transferCommandPool;
    ve::TransferCommandBuffer m_transferCommandBuffer;

    inline static constexpr std::uint32_t s_maxFramesInFlight{ 2U };
    ve::VertexBuffer m_vertexBuffer;
    ve::IndexBuffer m_indexBuffer;
    std::array< ve::UniformBuffer, s_maxFramesInFlight > m_uniformBuffers{
        ve::UniformBuffer{ m_memoryAllocator, sizeof( UniformBufferData ) },
        ve::UniformBuffer{ m_memoryAllocator, sizeof( UniformBufferData ) } };

    ve::DescriptorSetLayout m_descriptorSetLayout;
    ve::DescriptorPool m_descriptorPool;
    std::vector< vk::DescriptorSet > m_descriptorSets;

    std::array< vk::Semaphore, s_maxFramesInFlight > m_imageAvailableSemaphores{};
    std::array< vk::Semaphore, s_maxFramesInFlight > m_renderFinishedSemaphores{};
    std::array< vk::Fence, s_maxFramesInFlight > m_inFlightFences{};
    std::uint32_t m_currentFrame{};

    void createSyncObjects();
    void updateUniformBuffer();
    void configureDescriptorSets();
    void uploadBuffersData( std::span< Vertex > vertices, std::span< std::uint32_t > indices );

    std::optional< std::uint32_t > acquireNextImage();
    void draw( const std::uint32_t imageIndex );
    void present( const std::uint32_t imageIndex );
};

} // namespace ve
