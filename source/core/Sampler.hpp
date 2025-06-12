#pragma once

#include "LogicalDevice.hpp"

#include <fastgltf/core.hpp>

namespace ve {

class LogicalDevice;

class Sampler : public utils::NonCopyable {
public:
    Sampler( const ve::LogicalDevice& logicalDevice, const fastgltf::Sampler& gltfSampler );
    Sampler( const ve::LogicalDevice& logicalDevice, const float maxSamplerAnisotropy );

    ~Sampler();

    Sampler( ve::Sampler&& other ) noexcept : m_logicalDevice{ other.m_logicalDevice }, m_sampler{ other.m_sampler } {
        other.m_sampler = nullptr;
    }
    Sampler& operator=( ve::Sampler&& other ) = delete;

    const vk::Sampler get() const noexcept { return m_sampler; }

private:
    vk::Sampler m_sampler;
    const ve::LogicalDevice& m_logicalDevice;

    vk::Filter extractFilter( const fastgltf::Filter filter ) const noexcept;
    vk::SamplerMipmapMode extractMipmapMode( const fastgltf::Filter filter ) const noexcept;
};

} // namespace ve
