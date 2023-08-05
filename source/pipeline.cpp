#include "pipeline.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace VE {

Pipeline::Pipeline(std::string_view vertFilePath, std::string_view fragFilePath) {
    createGraphicsPipeline(vertFilePath, fragFilePath);
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

void Pipeline::createGraphicsPipeline(std::string_view vertFilePath, std::string_view fragFilePath) {
    const auto vertCode {std::move(readFile(vertFilePath))};
    const auto fragCode {std::move(readFile(fragFilePath))};

    std::cout << "vertex shader binary code size: " << std::size(vertCode) << '\n'
              << "fragment shader binary code size: " << std::size(fragCode) << '\n';
}

}