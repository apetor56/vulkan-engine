#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "MemoryAllocator.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Frame.hpp"
#include "Constants.hpp"
#include "command/CommandPool.hpp"
#include "command/GraphicsCommandBuffer.hpp"
#include "descriptor/DescriptorSetLayout.hpp"
#include "descriptor/DescriptorPool.hpp"

#include <functional>

namespace ve {

class Engine {
public:
    Engine();
    ~Engine();

    void run();

private:
    using Frames = std::array< std::optional< ve::Frame >, g_maxFramesInFlight >;

    ve::VulkanInstance m_vulkanInstance{};
    ve::Window m_window;
    ve::PhysicalDevice m_physicalDevice;
    ve::LogicalDevice m_logicalDevice;
    ve::MemoryAllocator m_memoryAllocator;
    ve::Swapchain m_swapchain;
    std::optional< ve::Pipeline > m_pipeline;
    ve::CommandPool< ve::GraphicsCommandBuffer > m_graphicsCommandPool;
    ve::Fence m_immediateSubmitFence;
    ve::GraphicsCommandBuffer m_immediateBuffer;
    ve::CommandPool< ve::TransferCommandBuffer > m_transferCommandPool;
    ve::TransferCommandBuffer m_transferCommandBuffer;
    ve::VertexBuffer m_vertexBuffer;
    ve::IndexBuffer m_indexBuffer;
    ve::DescriptorSetLayout m_descriptorSetLayout;
    std::optional< ve::DescriptorPool > m_descriptorPool;
    Frames m_frames;
    Frames::iterator m_currentFrameIt{ nullptr };
    std::optional< ve::Image > m_textureImage{};
    vk::Sampler m_sampler;

    void createFramesResoures();
    void updateUniformBuffer();
    void configureDescriptorSets();
    void uploadBuffersData( std::span< Vertex > vertices, std::span< std::uint32_t > indices );
    void prepareTexture();
    void createTextureSampler();

    std::optional< std::uint32_t > acquireNextImage();
    void draw( const std::uint32_t imageIndex );
    void present( const std::uint32_t imageIndex );

    template < std::derived_from< ve::BaseCommandBuffer > CommandBuffer_T >
    void immediateSubmit( const std::function< void( CommandBuffer_T command ) >& function ) {
        const auto logicalDeviceHandler{ m_logicalDevice.getHandler() };
        logicalDeviceHandler.resetFences( m_immediateSubmitFence.get() );
        m_immediateBuffer.reset();

        CommandBuffer_T command{ m_immediateBuffer };
        command.begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
        function( command );
        command.end();

        const auto commandHanlder{ command.getHandler() };
        vk::SubmitInfo submitInfo{};
        submitInfo.sType              = vk::StructureType::eSubmitInfo;
        submitInfo.commandBufferCount = 1U;
        submitInfo.pCommandBuffers    = &commandHanlder;

        m_logicalDevice.getQueue( ve::QueueType::eGraphics ).submit( submitInfo, m_immediateSubmitFence.get() );
        [[maybe_unused]] const auto waitForFencesResult{
            logicalDeviceHandler.waitForFences( m_immediateSubmitFence.get(), g_waitForAllFences, g_timeoutOff ) };
    }
};

} // namespace ve
