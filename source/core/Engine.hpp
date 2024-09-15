#pragma once

#include "VulkanInstance.hpp"
#include "Window.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "Swapchain.hpp"
#include "Pipeline.hpp"
#include "Buffer.hpp"

#include "command/CommandPool.hpp"
#include "command/GraphicsCommandBuffer.hpp"

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
    ve::Swapchain m_swapchain;
    ve::Pipeline m_pipeline;
    ve::CommandPool< ve::FamilyType::eGraphics > m_graphicsCommandPool;
    ve::VertexBuffer m_vertexBuffer;

    inline static constexpr std::uint32_t s_maxFramesInFlight{ 2U };
    inline static constexpr std::uint64_t s_timeoutOff{ std::numeric_limits< std::uint64_t >::max() };
    std::vector< ve::GraphicsCommandBuffer > m_commandBuffers;
    std::array< vk::Semaphore, s_maxFramesInFlight > m_imageAvailableSemaphores{};
    std::array< vk::Semaphore, s_maxFramesInFlight > m_renderFinishedSemaphores{};
    std::array< vk::Fence, s_maxFramesInFlight > m_inFlightFences{};
    std::uint32_t m_currentFrame{};

    void createSyncObjects();

    std::optional< std::uint32_t > acquireNextImage();
    void draw( const std::uint32_t imageIndex );
    void present( const std::uint32_t imageIndex );
};

} // namespace ve
