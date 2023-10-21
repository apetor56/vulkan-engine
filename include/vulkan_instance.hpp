#pragma once

#include "debug_messenger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

namespace VE {

class VulkanInstance {
public:
    VulkanInstance();

    ~VulkanInstance();

    void createVulkanInstance();

    std::vector<const char*> getRequiredInstanceExtensions() const;

    VkInstance get() const;

    void showAllSupportedExtensions() const;

private:
    VkInstance m_instance;

    #ifndef NDEBUG
        DebugMessenger m_debugMessenger{};
    #endif
};

}