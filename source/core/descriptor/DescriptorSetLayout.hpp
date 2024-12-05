#pragma once

#include "LogicalDevice.hpp"

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <unordered_map>

namespace ve {

class DescriptorSetLayout {
public:
    DescriptorSetLayout( const ve::LogicalDevice& logicalDevice ) noexcept;
    ~DescriptorSetLayout();

    void addBinding( const std::uint32_t bindingPoint, const vk::DescriptorType descriptorType,
                     const vk::ShaderStageFlags shaderStage, const std::uint32_t descriptorCount = 1U );

    void create();
    vk::DescriptorSetLayout get() const noexcept;

private:
    std::unordered_map< std::uint32_t, vk::DescriptorSetLayoutBinding > m_descriptorBindings;
    vk::DescriptorSetLayout m_layout;
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve