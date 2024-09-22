#include "DescriptorSetLayout.hpp"

#include <ranges>

namespace ve {

DescriptorSetLayout::DescriptorSetLayout( const ve::LogicalDevice& logicalDevice ) noexcept
    : m_logicalDevice{ logicalDevice } {}

DescriptorSetLayout ::~DescriptorSetLayout() {
    m_logicalDevice.getHandler().destroyDescriptorSetLayout( m_layout );
}

void DescriptorSetLayout::addBinding( const std::uint32_t bindingPoint, const vk::DescriptorType descriptorType,
                                      const vk::ShaderStageFlags shaderStage, const std::uint32_t descriptorCount ) {
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding         = bindingPoint;
    layoutBinding.descriptorType  = descriptorType;
    layoutBinding.stageFlags      = shaderStage;
    layoutBinding.descriptorCount = descriptorCount;

    m_descriptorBindings.emplace( bindingPoint, layoutBinding );
}

void DescriptorSetLayout::create() {
    const auto bindingsDataView{ m_descriptorBindings | std::views::values };
    const std::vector< vk::DescriptorSetLayoutBinding > bindingsData{ std::begin( bindingsDataView ),
                                                                      std::end( bindingsDataView ) };

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layoutInfo.bindingCount = static_cast< std::uint32_t >( std::size( m_descriptorBindings ) );
    layoutInfo.pBindings    = std::data( bindingsData );

    m_layout = m_logicalDevice.getHandler().createDescriptorSetLayout( layoutInfo );
}

vk::DescriptorSetLayout DescriptorSetLayout::getHandler() const noexcept {
    return m_layout;
}

} // namespace ve