#include "PhysicalDevice.h"

#include <algorithm>
#include <iostream>
#include "string.h"

namespace Render
{

const std::vector<const char*> PhysicalDevice::DeviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
    VK_EXT_SHADER_OBJECT_EXTENSION_NAME
};

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device)
: m_device(device)
, m_suitable(false)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(m_device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(m_device, &deviceFeatures);

    VkPhysicalDevicePushDescriptorProperties pushDescriptorProperties = {};
    pushDescriptorProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    deviceProperties2.pNext = &pushDescriptorProperties;
    vkGetPhysicalDeviceProperties2(m_device, &deviceProperties2);

    std::cout << deviceProperties.deviceName << std::endl;

    m_suitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                checkDeviceExtensionSupport() &&
                checkQueueFamilies() &&
                pushDescriptorProperties.maxPushDescriptors > 0 &&
                deviceFeatures.geometryShader &&
                deviceFeatures.samplerAnisotropy;
}

bool PhysicalDevice::checkDeviceExtensionSupport()
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, availableExtensions.data());

    for(const char * requiredExtension : DeviceExtensions)
    {
        bool found = false;

        for(const auto& extension : availableExtensions)
        {
            if(strcmp(requiredExtension, extension.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found) return false;
    }

    return true;
}

bool PhysicalDevice::checkQueueFamilies()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_graphicsFamily = i;
            m_presentationFamily = i;
            return true;
        }

        i++;
    }

    return false;
}

bool PhysicalDevice::findPresentationFamily(VkSurfaceKHR surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, queueFamilies.data());

    bool presentationQueueSupport = false;

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 surfaceSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_device, i, surface, &surfaceSupport);

        if (surfaceSupport)
        {
            m_presentationFamily = i;
            return true;
        }

        i++;
    }

    return false;
}

void PhysicalDevice::getSwapChainSupportInfo(VkSurfaceKHR surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device, surface, &m_surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_device, surface, &formatCount, nullptr);

    if(formatCount != 0)
    {
        m_surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_device, surface, &formatCount, m_surfaceFormats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &presentModeCount, nullptr);

    if(presentModeCount != 0)
    {
        m_presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &presentModeCount, m_presentModes.data());
    }
}

uint32_t PhysicalDevice::swapChainImageCount()
{
    uint32_t imageCount = m_surfaceCapabilities.minImageCount + 1;

    if(m_surfaceCapabilities.maxImageCount > 0 && imageCount > m_surfaceCapabilities.maxImageCount)
        imageCount = m_surfaceCapabilities.maxImageCount;

    return imageCount;
}

VkSurfaceFormatKHR PhysicalDevice::chooseSwapSurfaceFormat()
{
    for(const auto& surfaceFormat : m_surfaceFormats)
    {
        if(surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormat;
        }
    }

    return m_surfaceFormats[0];
}

VkPresentModeKHR PhysicalDevice::getSurfacePresentMode(VkSurfaceKHR surface)
{
    std::vector<VkPresentModeKHR> presentModes;

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &presentModeCount, presentModes.data());
    }

    for(const auto& presentMode : presentModes)
    {
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR) return presentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

} // namespace Render