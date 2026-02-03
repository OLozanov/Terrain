#pragma once

#include <vulkan/vulkan.h>
#include "VulkanInstance.h"

namespace Render
{

class Bitmap
{
    VkFormat m_format;
    VkImageUsageFlags m_usage;

    VkDeviceMemory m_imageMemory;
    VkImage m_image;
    VkImageView m_imageView;

    VkImageLayout m_layout;

private:
    VkImageLayout transitLayout(VkImageLayout newLayout);

public:
    Bitmap(VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT);
    ~Bitmap();

    static VkImageAspectFlags GetAspectMask(VkFormat format);

    void reset();
    void reset(uint32_t width, uint32_t height);

    void setLayout(VkImageLayout layout);

    operator VkImage() const { return m_image; }
    operator VkImageView() const { return m_imageView; }

    friend class CommandList;
};

} // namespace Render