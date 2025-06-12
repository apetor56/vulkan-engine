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

} // namespace ve

namespace ve::gltf {

struct Material {
    ve::Material data;
};

struct MetalicRoughness {
    MetalicRoughness( const ve::LogicalDevice& logicalDevice )
        : m_logicalDevice{ logicalDevice }, descriptorWriter{ logicalDevice } {}

    struct Constants {
        glm::vec4 colorFactors{};
        glm::vec4 metalicRoughnessFactors{};
        glm::vec4 extraPadding[ 14 ];
    };

    struct Resources {
        vk::ImageView colorImageView;
        vk::ImageView metalicRoughnessImageView;
        vk::Sampler colorSampler;
        vk::Sampler metalicRoughnessSampler;
        vk::Buffer dataBuffer;
        uint32_t dataBufferOffset;
    };

    void buildPipelines( const ve::DescriptorSetLayout& layout, const ve::RenderPass& renderPass );
    void clearResources();
    ve::Material writeMaterial( const ve::Material::Type materialType, const Resources& resources,
                                ve::DescriptorAllocator& descriptorAllocator );

    ve::DescriptorWriter descriptorWriter;
    std::optional< ve::Pipeline > opaquePipeline;
    std::optional< ve::Pipeline > transparentPipeline;
    std::optional< ve::PipelineLayout > pipelineLayout;
    std::optional< ve::DescriptorSetLayout > desMaterialLayout;

private:
    const ve::LogicalDevice& m_logicalDevice;
};

} // namespace ve::gltf
