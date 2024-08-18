#include "ShaderModule.hpp"

#include <fstream>
#include <filesystem>

namespace ve {

ShaderModule::ShaderModule( std::string_view shaderBinaryPath, const ve::LogicalDevice& logicalDevice )
    : m_logicalDevice{ logicalDevice } {
    const auto& shaderCode{ readShaderBinary( shaderPath ) };
    createShaderModule( shaderCode );
}

ShaderModule::~ShaderModule() {
    m_logicalDevice.getHandler().destroyShaderModule( m_shaderModule );
}

std::vector< std::byte > ShaderModule::getShaderBinaryCode( std::string_view shaderBinaryPath ) const {
    std::ifstream binaryFile{ shaderBinaryPath.data(), std::ios::binary };
    if ( !binaryFile.is_open() )
        throw std::runtime_error( "failed to open shader binary file" );

    const auto binarySize{ std::filesystem::file_size( shaderBinaryPath ) };
    std::vector< std::byte > shaderBinaryCode( binarySize );
    binaryFile.read( reinterpret_cast< char * >( shaderBinaryCode.data() ), binarySize );
    binaryFile.close();

    return shaderBinaryCode;
}

void ShaderModule::createShaderModule( const std::vector< std::byte >& shaderByteCode ) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = std::size( shaderByteCode );
    createInfo.pCode    = reinterpret_cast< const std::uint32_t    *>( shaderByteCode.data() );

    m_shaderModule = m_logicalDevice.getHandler().createShaderModule( createInfo );
}

vk::ShaderModule ShaderModule::getHandler() const noexcept {
    return m_shaderModule;
}

} // namespace ve
