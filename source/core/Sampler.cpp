#include "Sampler.hpp"

#include <spdlog/spdlog.h>

namespace ve {

Sampler::Sampler( const ve::LogicalDevice& logicalDevice, const fastgltf::Sampler& gltfSampler )
    : m_logicalDevice{ logicalDevice } {
    vk::SamplerCreateInfo createInfo{};
    createInfo.sType      = vk::StructureType::eSamplerCreateInfo;
    createInfo.pNext      = nullptr;
    createInfo.maxLod     = vk::LodClampNone;
    createInfo.minLod     = 0U;
    createInfo.magFilter  = extractFilter( gltfSampler.magFilter.value_or( fastgltf::Filter::Nearest ) );
    createInfo.minFilter  = extractFilter( gltfSampler.minFilter.value_or( fastgltf::Filter::Nearest ) );
    createInfo.mipmapMode = extractMipmapMode( gltfSampler.minFilter.value_or( fastgltf::Filter::Nearest ) );

    m_sampler = logicalDevice.get().createSampler( createInfo );
}

Sampler::Sampler( const ve::LogicalDevice& logicalDevice, const float maxSamplerAnisotropy )
    : m_logicalDevice{ logicalDevice } {
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter               = vk::Filter::eLinear;
    samplerInfo.minFilter               = vk::Filter::eLinear;
    samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable        = vk::True;
    samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable           = vk::False;
    samplerInfo.compareOp               = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias              = 0.0F;
    samplerInfo.minLod                  = 0.0F;
    samplerInfo.maxLod                  = 0.0F;
    samplerInfo.maxAnisotropy           = maxSamplerAnisotropy;

    m_sampler = logicalDevice.get().createSampler( samplerInfo );
}

Sampler::~Sampler() {
    m_logicalDevice.get().destroySampler( m_sampler );
}

vk::Filter Sampler::extractFilter( const fastgltf::Filter filter ) const noexcept {
    switch ( filter ) {
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
        return vk::Filter::eNearest;

    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return vk::Filter::eLinear;
    }
}

vk::SamplerMipmapMode Sampler::extractMipmapMode( const fastgltf::Filter filter ) const noexcept {
    switch ( filter ) {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
        return vk::SamplerMipmapMode::eNearest;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return vk::SamplerMipmapMode::eLinear;
    }
}
} // namespace ve
