#pragma once

#include <string_view>
#include <vector>

namespace VE {

class Pipeline {
public:
    Pipeline(std::string_view vertFilePath, std::string_view fragFilePath);

private:
    static std::vector<char> readFile(std::string_view filePath);
    void createGraphicsPipeline(std::string_view vertFilePath, std::string_view fragFilePath);
};

};