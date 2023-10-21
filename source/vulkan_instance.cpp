#include "vulkan_instance.hpp"

#include <iostream>

namespace VE {

VulkanInstance::VulkanInstance() {
    glfwInit();
    createVulkanInstance();

    #ifndef NDEBUG
        m_debugMessenger.init(m_instance);
    #endif
}

VulkanInstance::~VulkanInstance() {
    #ifndef NDEBUG
        m_debugMessenger.destroy(m_instance);
    #endif
    
    vkDestroyInstance(m_instance, nullptr);
    glfwTerminate();
}

void VulkanInstance::createVulkanInstance() {
    #ifndef NDEBUG
        showAllSupportedExtensions();
        
        if(!m_debugMessenger.areValidationLayersSupported()) {
            throw std::runtime_error("validation layers requested but not available");
        }
    #endif

    // optional
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = "Vulkan learning";
    appInfo.pEngineName        = nullptr;
    appInfo.apiVersion         = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

    const auto extensions { getRequiredInstanceExtensions() };

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    #ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        const auto& validationLayers { m_debugMessenger.getLayers() };
        instanceInfo.enabledLayerCount   = static_cast<uint32_t>(std::size(validationLayers));
        instanceInfo.ppEnabledLayerNames = validationLayers.data();

        m_debugMessenger.populateCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    #else
        instanceInfo.enabledLayerCount = 0u;
        instanceInfo.pNext             = nullptr;
    #endif

    if(vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vulkan instance");
    }
}

std::vector<const char*> VulkanInstance::getRequiredInstanceExtensions() const {
    uint32_t glfwExtensionCount{};
    const char **glfwExtensions { glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    #ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    return extensions;
}

VkInstance VulkanInstance::get() const {
    return m_instance;
}

void VulkanInstance::showAllSupportedExtensions() const {
    uint32_t extensionCount{};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "all available vulkan instance extensions: \n";
    for(const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

}