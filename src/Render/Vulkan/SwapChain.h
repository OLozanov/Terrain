#pragma once

#include <vulkan/vulkan.h>

#include "PhysicalDevice.h"
#include "FrameBuffer.h"

namespace Render
{

class SwapChain
{
public:
    SwapChain() {}
    SwapChain(VkSurfaceKHR surface);
    ~SwapChain();

    uint32_t acquireBuffer();

    VkImage image(uint32_t i) { return m_images[i]; }
    VkImageView colorBuffer(uint32_t i) { return m_imageViews[i]; }

    const VkExtent2D& frameExtent() const { return m_imageExtent; }

    void present();

private:
    void createDepthBuffer(const VkExtent2D& imageExtent);

private:
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;

    VkExtent2D m_imageExtent;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    VkSemaphore m_imageAvailableSemaphore;

    uint32_t m_imageIndex;
};

} // namespace Render