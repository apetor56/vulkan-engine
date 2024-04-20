#pragma once

#include "DebugMessenger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

namespace ve {

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    VulkanInstance( const VulkanInstance& other ) = delete;
    VulkanInstance( VulkanInstance&& other )      = delete;

    VulkanInstance& operator=( const VulkanInstance& other ) = delete;
    VulkanInstance& operator=( VulkanInstance&& other )      = delete;

    void createVulkanInstance();
    std::vector< const char * > getRequiredInstanceExtensions() const;

    VkInstance get() const noexcept;
    void showAllSupportedExtensions() const;

private:
    VkInstance m_instance;

#ifndef NDEBUG
    DebugMessenger m_debugMessenger{};
#endif
};

} // namespace ve