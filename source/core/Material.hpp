#pragma once

#include "Pipeline.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "descriptor/DescriptorWriter.hpp"

namespace ve {

class DescriptorSetLayout;
class DescriptorAllocator;
class RenderPass;

struct Material {
    enum class Type { eMainColor, eTransparent, eOther };

    std::optional< ve::Pipeline > pipeline;
    vk::DescriptorSet descriptorSet;
    const Type type{};
};

struct GltfMetalicRoughness {
    struct Constants {
        const glm::vec4 colorFactors{};
        const glm::vec4 metalicRoughnessFactors{};
        const glm::vec4 extraPadding[ 14 ];
    };

    struct Resources {
        const ve::Image colorImage;
        const ve::Image metalicRoughnessImage;
        const vk::Sampler colorSampler;
        const vk::Sampler metalicRoughnessSampler;
        const ve::UniformBuffer dataBuffer;
        const uint32_t offset;
    };

    void buildPipelines( const ve::DescriptorSetLayout& layout, const ve::LogicalDevice& logicalDevice,
                         const ve::RenderPass& renderPass );
    void clearResources( const ve::LogicalDevice& logicalDevice );
    Material writeMaterial( const ve::LogicalDevice& logicalDevice, Material::Type materialType,
                            const Resources& resources, const ve::DescriptorAllocator& descriptorAllocator );

    std::optional< ve::DescriptorWriter > descriptorWriter;
    std::optional< ve::Pipeline > opaquePipeline;
    std::optional< ve::Pipeline > transparentPipeline;
    std::optional< ve::PipelineLayout > meshLayout;
};

} // namespace ve
