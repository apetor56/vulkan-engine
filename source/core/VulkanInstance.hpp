#pragma once

#include <vulkan/vulkan.hpp>
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

    vk::Instance get() const noexcept;

private:
    vk::Instance m_instance;

    void createVulkanInstance();
    void showAllSupportedExtensions() const;
    ve::extentions getRequiredInstanceExtensions() const;
};

} // namespace ve
