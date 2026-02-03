#pragma once

#include <vulkan/vulkan.h>

struct Image
{
    uint32_t width;
    uint32_t height;
    size_t mipmaps;

    void* data = nullptr;

    VkFormat format = VK_FORMAT_UNDEFINED;

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;

    size_t size();

    Image() = default;
    ~Image();

    operator VkImageView() const { return imageView; }
};

size_t pixelsize(VkFormat format);

void BuildMipmaps(uint8_t* data, uint32_t width, uint32_t height, VkFormat format, size_t mipmaps);

Image* LoadBMP(const char* filename);
Image* LoadPNG(const char* filename, bool mipmaps = true, bool rawdata = false);
Image* LoadImage(const char* filename);