#pragma once

#include "LogicalDevice.hpp"

#include <fastgltf/core.hpp>

namespace ve {

class LogicalDevice;

class Sampler : public utils::NonCopyable {
public:
    Sampler( const ve::LogicalDevice& logicalDevice, const fastgltf::Sampler& gltfSampler );
    Sampler( const ve::LogicalDevice& logicalDevice );
    Sampler( const ve::LogicalDevice& logicalDevice, const vk::SamplerCreateInfo& info );

    ~Sampler();

    Sampler( ve::Sampler&& other ) noexcept;
    Sampler& operator=( ve::Sampler&& other ) = delete;

    const vk::Sampler get() const noexcept { return m_sampler; }

private:
    vk::Sampler m_sampler;
    const ve::LogicalDevice& m_logicalDevice;

    vk::Filter extractFilter( const fastgltf::Filter filter ) const noexcept;
    vk::SamplerMipmapMode extractMipmapMode( const fastgltf::Filter filter ) const noexcept;

    vk::SamplerCreateInfo getDefaultInfo() const noexcept {
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter               = vk::Filter::eLinear;
        samplerInfo.minFilter               = vk::Filter::eLinear;
        samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
        samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
        samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = vk::False;
        samplerInfo.compareEnable           = vk::False;
        samplerInfo.compareOp               = vk::CompareOp::eAlways;
        samplerInfo.mipLodBias              = 0.0F;
        samplerInfo.minLod                  = 0.0F;
        samplerInfo.maxLod                  = vk::LodClampNone;
        samplerInfo.anisotropyEnable        = vk::False;
        samplerInfo.maxAnisotropy           = 0.0F;

        const auto physicalDeviceProperties{ m_logicalDevice.getParentPhysicalDevice().get().getProperties() };
        samplerInfo.anisotropyEnable = vk::True;
        samplerInfo.maxAnisotropy    = physicalDeviceProperties.limits.maxSamplerAnisotropy;

        return samplerInfo;
    }
};

} // namespace ve
