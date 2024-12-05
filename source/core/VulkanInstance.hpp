#pragma once

#include "DebugMessenger.hpp"

#include "utils/NonCopyable.hpp"
#include "utils/NonMovable.hpp"

#include <vulkan/vulkan.hpp>

#include <optional>

namespace ve {

class VulkanInstance : public utils::NonCopyable,
                       public utils::NonMovable {
public:
    VulkanInstance();
    ~VulkanInstance();

    vk::Instance get() const noexcept { return m_instance; }

private:
    vk::Instance m_instance;
    std::optional< ve::DebugMessenger > m_debugMessenger;

    void createVulkanInstance();
    std::vector< const char * > getRequiredInstanceExtensions() const;
};

} // namespace ve
