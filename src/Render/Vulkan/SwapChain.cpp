#include "SwapChain.h"
#include "VulkanInstance.h"

#include <stdexcept>

namespace Render
{

SwapChain::SwapChain(VkSurfaceKHR surface)
: m_surface(surface)
{
    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    vkInstance.swapChainSupportInfo(surface);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(vkInstance.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }

    VkExtent2D imageExtent;
    m_swapchain = vkInstance.createSwapChain(surface, imageExtent);

    uint32_t imageCount;

    vkGetSwapchainImagesKHR(vkInstance.device(), m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(vkInstance.device(), m_swapchain, &imageCount, m_images.data());

    m_imageViews.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        // Image views
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkInstance.device(), &createInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }

    m_imageExtent = imageExtent;

    vkInstance.transitImageState(m_images, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

SwapChain::~SwapChain()
{
    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    vkDestroySemaphore(vkInstance.device(), m_imageAvailableSemaphore, nullptr);

    for (size_t i = 0; i < m_imageViews.size(); i++)
    {
        vkDestroyImageView(vkInstance.device(), m_imageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(vkInstance.device(), m_swapchain, nullptr);
    vkDestroySurfaceKHR(vkInstance, m_surface, nullptr);
}

uint32_t SwapChain::acquireBuffer()
{
    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    vkAcquireNextImageKHR(vkInstance.device(), m_swapchain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &m_imageIndex);

    return m_imageIndex;
}

void SwapChain::present()
{
    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = { m_imageAvailableSemaphore };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(vkInstance.presentQueue(), &presentInfo);
    vkQueueWaitIdle(vkInstance.presentQueue());
}

} // namespace Render