#include "command_buffer.hpp"
#include "queue_family_indices.hpp"

namespace VE {

CommandBuffer::CommandBuffer(std::shared_ptr<PhysicalDevice> physicalDevice,
                             std::shared_ptr<LogicalDevice> logicalDevice,
                             std::shared_ptr<Swapchain> swapchain,
                             std::shared_ptr<Pipeline> pipeline) : m_physicalDevice { physicalDevice },
                                                                    m_logicalDevice { logicalDevice },
                                                                    m_swapchain { swapchain },
                                                                    m_pipeline { pipeline } {
    createCommandPool();
    createCommandBuffer();

    for(uint32_t imageIndex{}; imageIndex < m_swapchain->getImagesCount(); imageIndex++) {
        record(imageIndex);
    }
}

CommandBuffer::~CommandBuffer() {
    vkDestroyCommandPool(m_logicalDevice->getHandle(), m_commandPool, nullptr);
}

void CommandBuffer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices { QueueFamilyIndices::findQueueFamilies(m_physicalDevice->getHandle(),
                                                                                  m_physicalDevice->getSurface()) };

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_logicalDevice->getHandle(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CommandBuffer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_logicalDevice->getHandle(), &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandBuffer::record(const uint32_t imageIndex) const {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0u;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapchain->getRenderpass();
    renderPassInfo.framebuffer = m_swapchain->getFrambuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchain->getExtent();

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getHandle());

        const auto& viewport { m_pipeline->getViewport() };
        vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
        const auto& scissor { m_pipeline->getScissor() };
        vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
    vkCmdEndRenderPass(m_commandBuffer);

    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }
}

}