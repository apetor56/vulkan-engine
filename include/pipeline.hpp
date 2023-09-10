#pragma once

#include "device.hpp"
#include <string_view>
#include <vector>

namespace VE {

struct PipelineConfigInfo {};

class Pipeline {
public:
    Pipeline(Device& device,
             std::string_view vertFilePath,
             std::string_view fragFilePath,
             const PipelineConfigInfo& configInfo);

    ~Pipeline() {}

    Pipeline(const Pipeline& other) = delete;
    Pipeline& operator=(const Pipeline other) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

private:
    Device& m_device;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;

    static std::vector<char> readFile(std::string_view filePath);

    void createGraphicsPipeline(std::string_view vertFilePath,
                                std::string_view fragFilePath,
                                const PipelineConfigInfo& configInfo);

    void createShaderModule(const std::vector<char> code, VkShaderModule *shaderModule);
};

};