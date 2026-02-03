#include "Bitmap.h"
#include "Render/Vulkan/VulkanInstance.h"

#include <stdexcept>

namespace Render
{

Bitmap::Bitmap(VkFormat format, VkImageUsageFlags usage)
: m_format(format)
, m_usage(usage)
, m_imageMemory(VK_NULL_HANDLE)
, m_image(VK_NULL_HANDLE)
, m_imageView(VK_NULL_HANDLE)
{
}

Bitmap::~Bitmap()
{
    reset();
}

void Bitmap::reset()
{
    if (m_image != VK_NULL_HANDLE)
    {
        VulkanInstance& vkInstance = VulkanInstance::GetInstance();

        vkDestroyImageView(vkInstance.device(), m_imageView, nullptr);
        vkDestroyImage(vkInstance.device(), m_image, nullptr);
        vkFreeMemory(vkInstance.device(), m_imageMemory, nullptr);
    }
}

VkImageAspectFlags Bitmap::GetAspectMask(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return VK_IMAGE_ASPECT_COLOR_BIT;
}

void Bitmap::reset(uint32_t width, uint32_t height)
{
    reset();

    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = m_usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if(vkCreateImage(vkInstance.device(), &imageInfo, nullptr, &m_image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkInstance.device(), m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vkInstance.detectMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(vkAllocateMemory(vkInstance.device(), &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(vkInstance.device(), m_image, m_imageMemory, 0);

    // ImageView
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = GetAspectMask(m_format);
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vkInstance.device(), &createInfo, nullptr, &m_imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image views!");
    }

    m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageLayout Bitmap::transitLayout(VkImageLayout newLayout)
{
    VkImageLayout oldLayout = m_layout;
    m_layout = newLayout;

    return oldLayout;
}

void Bitmap::setLayout(VkImageLayout layout)
{
    if (m_layout != layout)
    {
        VulkanInstance& vkInstance = VulkanInstance::GetInstance();
        vkInstance.transitImageState(m_image, m_layout, layout);

        m_layout = layout;
    }
}

} // namespace Render