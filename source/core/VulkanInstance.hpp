#pragma once

#include "DebugMessenger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

    void createVulkanInstance();
    const extentions getRequiredInstanceExtensions() const;
    VkInstance get() const noexcept;
    void showAllSupportedExtensions() const;

private:
    VkInstance m_instance;

#ifndef NDEBUG
    ve::DebugMessenger m_debugMessenger{};
#endif
};

} // namespace ve