#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Render
{

class PhysicalDevice
{
public:
    static const std::vector<const char*> DeviceExtensions;

private:
    VkPhysicalDevice m_device;

    bool m_suitable;

    uint32_t m_graphicsFamily;
    uint32_t m_presentationFamily;

    VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
    std::vector<VkPresentModeKHR> m_presentModes;

    bool checkDeviceExtensionSupport();
    bool checkQueueFamilies();

public:

    PhysicalDevice(VkPhysicalDevice device);

    bool isSuitable() { return m_suitable; }

    operator VkPhysicalDevice& () { return m_device; }

    uint32_t graphicsFamilyIndex() { return m_graphicsFamily; }
    uint32_t presentationFamilyIndex() { return m_presentationFamily; }

    bool findPresentationFamily(VkSurfaceKHR surface);

    uint32_t swapChainImageCount();
    VkSurfaceTransformFlagBitsKHR currentTransform() { return m_surfaceCapabilities.currentTransform; }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkPresentModeKHR getSurfacePresentMode(VkSurfaceKHR surface);

    void getSwapChainSupportInfo(VkSurfaceKHR surface);
};

} // namespace Render
