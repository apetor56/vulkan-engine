#pragma once

#include "DebugMessenger.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <optional>

namespace ve {

using extentions = std::vector< const char * >;

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    VulkanInstance( const VulkanInstance& other ) = delete;
    VulkanInstance( VulkanInstance&& other )      = delete;

    VulkanInstance& operator=( const VulkanInstance& other ) = delete;
    VulkanInstance& operator=( VulkanInstance&& other )      = delete;

    vk::Instance get() const noexcept;

private:
    vk::Instance m_instance;
    std::optional< ve::DebugMessenger > m_debugMessenger;

    void createVulkanInstance();
    void showAllSupportedExtensions() const;
    ve::extentions getRequiredInstanceExtensions() const;
};

} // namespace ve
