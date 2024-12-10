#include "ShaderModule.hpp"

#include <fstream>
#include <filesystem>

namespace ve {

ShaderModule::ShaderModule( const std::filesystem::path& shaderBinaryPath, const ve::LogicalDevice& logicalDevice )
    : m_logicalDevice{ logicalDevice } {
    createShaderModule( getShaderBinaryCode( shaderBinaryPath ) );
}

ShaderModule::~ShaderModule() {
    m_logicalDevice.get().destroyShaderModule( m_shaderModule );
}

std::vector< std::byte > ShaderModule::getShaderBinaryCode( const std::filesystem::path& shaderBinaryPath ) const {
    std::ifstream binaryFile{ shaderBinaryPath.string(), std::ios::binary };
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
    createInfo.pCode    = reinterpret_cast< const std::uint32_t * >( shaderByteCode.data() );

    m_shaderModule = m_logicalDevice.get().createShaderModule( createInfo );
}

vk::ShaderModule ShaderModule::get() const noexcept {
    return m_shaderModule;
}

} // namespace ve
