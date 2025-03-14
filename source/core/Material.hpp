#pragma once

#include "Pipeline.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "descriptor/DescriptorWriter.hpp"

namespace ve {

class Engine;
class DescriptorAllocator;

struct PushConstants {
    glm::mat4 worldMatrix;
};

struct Material {
    enum class Type { eMainColor, eTransparent, eOther };

    const ve::Pipeline& pipeline;
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

    void buildPipelines( const ve::Engine& engine, const ve::LogicalDevice& logicalDevice );
    void clearResources( const ve::LogicalDevice& logicalDevice );
    Material writeMaterial( const ve::LogicalDevice& logicalDevice, Material::Type materialType,
                            const Resources& resources, const ve::DescriptorAllocator& descriptorAllocator );

    const ve::DescriptorWriter descriptorWriter;
    const ve::Pipeline& opaquePipeline;
    const ve::Pipeline& transparentPipeline;
};

} // namespace ve
