#include "Material.hpp"
#include "ShaderModule.hpp"
#include "Mesh.hpp"
#include "Config.hpp"
#include "RenderPass.hpp"

#include "descriptor/DescriptorSetLayout.hpp"
#include "descriptor/DescriptorAllocator.hpp"

#include "utils/Common.hpp"

namespace ve::gltf {

void MetalicRoughness::buildPipelines( const ve::DescriptorSetLayout& layout, const ve::RenderPass& renderPass ) {
    const ve::ShaderModule meshVertexShader{ cfg::directory::shaderBinaries / "Mesh.vert.spv", m_logicalDevice };
    const ve::ShaderModule meshFragmentShader{ cfg::directory::shaderBinaries / "Mesh.frag.spv", m_logicalDevice };

    static constexpr vk::PushConstantRange range{ ve::PushConstants::defaultRange() };
    static constexpr vk::ShaderStageFlags shaderStages{ vk::ShaderStageFlagBits::eVertex |
                                                        vk::ShaderStageFlagBits::eFragment };

    desMaterialLayout.emplace( m_logicalDevice );
    desMaterialLayout->addBinding( 0U, vk::DescriptorType::eUniformBuffer, shaderStages );
    desMaterialLayout->addBinding( 1U, vk::DescriptorType::eCombinedImageSampler, shaderStages );
    desMaterialLayout->create();

    const std::array< vk::DescriptorSetLayout, 2U > layoutsVk{ layout.get(), desMaterialLayout->get() };
    auto meshLayoutInfo{ ve::PipelineLayout::defaultInfo() };
    meshLayoutInfo.pPushConstantRanges    = &range;
    meshLayoutInfo.pushConstantRangeCount = 1U;
    meshLayoutInfo.pSetLayouts            = std::data( layoutsVk );
    meshLayoutInfo.setLayoutCount         = utils::size( layoutsVk );
    pipelineLayout.emplace( m_logicalDevice, meshLayoutInfo );

    ve::PipelineBuilder builder{ m_logicalDevice };
    builder.setShaders( meshVertexShader, meshFragmentShader );
    builder.setLayout( pipelineLayout.value() );
    builder.disableBlending();
    opaquePipeline.emplace( builder, renderPass );

    builder.enableBlendingAdditive();
    builder.disableDepthWrite();
    transparentPipeline.emplace( builder, renderPass );
}

ve::Material MetalicRoughness::writeMaterial( const ve::Material::Type materialType, const Resources& resources,
                                              ve::DescriptorAllocator& descriptorAllocator ) {
    if ( !transparentPipeline.has_value() || !opaquePipeline.has_value() )
        throw std::runtime_error( "MetalicRoughness: pipeline not built" );

    const vk::DescriptorSet set{ descriptorAllocator.allocate( desMaterialLayout.value() ) };

    descriptorWriter.clear();
    descriptorWriter.writeBuffer( 0U, resources.dataBuffer, sizeof( Constants ), resources.dataBufferOffset,
                                  vk::DescriptorType::eUniformBuffer );
    descriptorWriter.writeImage( 1U, resources.colorImageView, vk::ImageLayout::eShaderReadOnlyOptimal,
                                 resources.colorSampler, vk::DescriptorType::eCombinedImageSampler );

    descriptorWriter.updateSet( set );

    if ( materialType == ve::Material::Type::eTransparent )
        return ve::Material{ .pipeline{ transparentPipeline.value() }, .descriptorSet{ set }, .type{ materialType } };

    if ( materialType == ve::Material::Type::eMainColor )
        return ve::Material{ .pipeline{ opaquePipeline.value() }, .descriptorSet{ set }, .type{ materialType } };

    throw std::runtime_error( "given material type not found" );
}

} // namespace ve::gltf
