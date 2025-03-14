#include "Material.hpp"
#include "ShaderModule.hpp"
#include "Mesh.hpp"
#include "Config.hpp"
#include "RenderPass.hpp"
#include "descriptor/DescriptorSetLayout.hpp"
#include "utils/Common.hpp"

namespace ve {

void GltfMetalicRoughness::buildPipelines( const ve::DescriptorSetLayout& layout,
                                           const ve::LogicalDevice& logicalDevice, const ve::RenderPass& renderPass ) {
    const ve::ShaderModule meshVertexShader{ cfg::directory::shaderBinaries / "Mesh.vert.spv", logicalDevice };
    const ve::ShaderModule meshFragmentShader{ cfg::directory::shaderBinaries / "Mesh.frag.spv", logicalDevice };

    static constexpr vk::PushConstantRange range{ ve::PushConstants::defaultRange() };
    static constexpr vk::ShaderStageFlags shaderStages{ vk::ShaderStageFlagBits::eVertex |
                                                        vk::ShaderStageFlagBits::eFragment };

    ve::DescriptorSetLayout materialLayout{ logicalDevice };
    materialLayout.addBinding( 0U, vk::DescriptorType::eUniformBuffer, shaderStages );
    materialLayout.addBinding( 1U, vk::DescriptorType::eCombinedImageSampler, shaderStages );
    materialLayout.addBinding( 2U, vk::DescriptorType::eCombinedImageSampler, shaderStages );
    materialLayout.create();

    const std::array< vk::DescriptorSetLayout, 2U > layoutsVk{ layout.get(), materialLayout.get() };
    auto meshLayoutInfo{ ve::PipelineLayout::defaultInfo() };
    meshLayoutInfo.pPushConstantRanges    = &range;
    meshLayoutInfo.pushConstantRangeCount = 1U;
    meshLayoutInfo.pSetLayouts            = std::data( layoutsVk );
    meshLayoutInfo.setLayoutCount         = utils::size( layoutsVk );
    meshLayout.emplace( logicalDevice, meshLayoutInfo );

    ve::PipelineBuilder builder{ logicalDevice };
    builder.setShaders( meshVertexShader, meshFragmentShader );
    builder.setLayout( meshLayout.value() );
    builder.disableBlending();
    opaquePipeline.emplace( builder, renderPass );

    builder.enableBlendingAdditive();
    builder.disableDepthWrite();
    transparentPipeline.emplace( builder, renderPass );
}

} // namespace ve
