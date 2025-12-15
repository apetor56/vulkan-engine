#include "Sampler.hpp"

#include <spdlog/spdlog.h>

namespace ve {

Sampler::Sampler( const ve::LogicalDevice& logicalDevice, const fastgltf::Sampler& gltfSampler )
    : m_logicalDevice{ logicalDevice } {
    vk::SamplerCreateInfo samplerInfo{ getDefaultInfo() };
    samplerInfo.magFilter  = extractFilter( gltfSampler.magFilter.value_or( fastgltf::Filter::Linear ) );
    samplerInfo.minFilter  = extractFilter( gltfSampler.minFilter.value_or( fastgltf::Filter::Linear ) );
    samplerInfo.mipmapMode = extractMipmapMode( gltfSampler.minFilter.value_or( fastgltf::Filter::Linear ) );

    m_sampler = logicalDevice.get().createSampler( samplerInfo );
}

Sampler::Sampler( const ve::LogicalDevice& logicalDevice ) : m_logicalDevice{ logicalDevice } {
    m_sampler = logicalDevice.get().createSampler( getDefaultInfo() );
}

Sampler::Sampler( ve::Sampler&& other ) noexcept
    : m_sampler{ other.m_sampler }, m_logicalDevice{ other.m_logicalDevice } {
    other.m_sampler = nullptr;
}

Sampler::Sampler( const ve::LogicalDevice& logicalDevice, const vk::SamplerCreateInfo& info )
    : m_logicalDevice{ logicalDevice } {
    m_sampler = m_logicalDevice.get().createSampler( info );
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
