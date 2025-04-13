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

    const ve::Pipeline& pipeline;
    vk::DescriptorSet descriptorSet;
    const Type type{};
};

struct GltfMaterial {
    Material data;
};

struct GltfMetalicRoughness {
    GltfMetalicRoughness( const ve::LogicalDevice& logicalDevice )
        : m_logicalDevice{ logicalDevice }, descriptorWriter{ logicalDevice } {}

    struct Constants {
        glm::vec4 colorFactors{};
        glm::vec4 metalicRoughnessFactors{};
        glm::vec4 extraPadding[ 14 ];
    };

    struct Resources {
        const vk::ImageView textureImageView;
        const vk::ImageView metalicRoughnessImageView;
        const vk::Sampler textureSampler;
        const vk::Sampler metalicRoughnessSampler;
        const vk::Buffer dataBuffer;
        const uint32_t dataBufferOffset;
    };

    void buildPipelines( const ve::DescriptorSetLayout& layout, const ve::RenderPass& renderPass );
    void clearResources();
    Material writeMaterial( const Material::Type materialType, const Resources& resources,
                            ve::DescriptorAllocator& descriptorAllocator );

    ve::DescriptorWriter descriptorWriter;
    std::optional< ve::Pipeline > opaquePipeline;
    std::optional< ve::Pipeline > transparentPipeline;
    std::optional< ve::PipelineLayout > pipelineLayout;
    std::optional< ve::DescriptorSetLayout > desMaterialLayout;

private:
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve
