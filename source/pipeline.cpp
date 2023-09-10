#include "pipeline.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace VE {

Pipeline::Pipeline(Device& device,
                   std::string_view vertFilePath,
                   std::string_view fragFilePath,
                   const PipelineConfigInfo& configInfo) : m_device(device) {
    createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

std::vector<char> Pipeline::readFile(std::string_view filePath) {
    std::ifstream file{filePath.data(), std::ios::ate | std::ios::binary};

    if(file.is_open() == false) {
        throw std::runtime_error("failed to open file: " + std::string{filePath.data()});
    }

    const size_t fileSize {static_cast<const size_t>(file.tellg())};
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void Pipeline::createGraphicsPipeline(std::string_view vertFilePath,
                                      std::string_view fragFilePath,
                                      const PipelineConfigInfo& configInfo) {
    const auto vertCode { std::move(readFile(vertFilePath)) };
    const auto fragCode { std::move(readFile(fragFilePath)) };

    std::cout << "vertex shader binary code size: " << std::size(vertCode) << '\n'
              << "fragment shader binary code size: " << std::size(fragCode) << '\n';
}

void Pipeline::createShaderModule(const std::vector<char> code, VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    createInfo.codeSize = std::size(code);

    if(vkCreateShaderModule(m_device.getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }
}

PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
    PipelineConfigInfo configInfo{};

    return configInfo;
}

}