#pragma once

#include "LogicalDevice.hpp"

#include <unordered_map>

namespace ve {

class DescriptorSetLayout : public utils::NonCopyable,
                            public utils::NonMovable {
public:
    DescriptorSetLayout( const ve::LogicalDevice& logicalDevice ) noexcept;
    ~DescriptorSetLayout();

    void addBinding( const uint32_t bindingPoint, const vk::DescriptorType descriptorType,
                     const vk::ShaderStageFlags shaderStage, const uint32_t descriptorCount = 1U );

    void create();
    vk::DescriptorSetLayout get() const noexcept;

private:
    std::unordered_map< uint32_t, vk::DescriptorSetLayoutBinding > m_descriptorBindings;
    vk::DescriptorSetLayout m_layout;
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve
