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

#include "command/CommandPool.hpp"
#include "command/GraphicsCommandBuffer.hpp"

#include "descriptor/DescriptorSetLayout.hpp"
#include "descriptor/DescriptorPool.hpp"

#include <functional>

namespace {
constexpr std::uint64_t g_timeoutOff{ std::numeric_limits< std::uint64_t >::max() };
constexpr auto g_waitForAllFences{ vk::True };
} // namespace

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
    ve::GraphicsCommandBuffer m_immediateBuffer;
    ve::CommandPool< ve::TransferCommandBuffer > m_transferCommandPool;
    ve::TransferCommandBuffer m_transferCommandBuffer;

    inline static constexpr std::uint32_t s_maxFramesInFlight{ 2U };
    ve::VertexBuffer m_vertexBuffer;
    ve::IndexBuffer m_indexBuffer;
    std::array< ve::UniformBuffer, s_maxFramesInFlight > m_uniformBuffers{
        ve::UniformBuffer{ m_memoryAllocator, sizeof( UniformBufferData ) },
        ve::UniformBuffer{ m_memoryAllocator, sizeof( UniformBufferData ) } };

    ve::DescriptorSetLayout m_descriptorSetLayout;
    std::optional< ve::DescriptorPool > m_descriptorPool;
    std::vector< vk::DescriptorSet > m_descriptorSets;

    std::array< vk::Semaphore, s_maxFramesInFlight > m_imageAvailableSemaphores{};
    std::array< vk::Semaphore, s_maxFramesInFlight > m_renderFinishedSemaphores{};
    std::array< vk::Fence, s_maxFramesInFlight > m_inFlightFences{};
    std::uint32_t m_currentFrame{};

    std::optional< ve::Image > m_textureImage{};
    vk::Sampler m_sampler;

    void createSyncObjects();
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
        logicalDeviceHandler.resetFences( m_immediateSubmitFence );
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

        m_logicalDevice.getQueue( ve::QueueType::eGraphics ).submit( submitInfo, m_immediateSubmitFence );
        [[maybe_unused]] const auto waitForFencesResult{
            logicalDeviceHandler.waitForFences( m_immediateSubmitFence, g_waitForAllFences, g_timeoutOff ) };
    }
};

} // namespace ve
