#include "Image.h"
#include "Render/Vulkan/VulkanInstance.h"

#include <iostream>

Image::~Image()
{
    Render::VulkanInstance& vkInstance = Render::VulkanInstance::GetInstance();

    if (image != VK_NULL_HANDLE)
    {
        vkDestroyImageView(vkInstance.device(), imageView, nullptr);
        vkDestroyImage(vkInstance.device(), image, nullptr);
        vkFreeMemory(vkInstance.device(), imageMemory, nullptr);
    }

    if (data) delete[] data;
}

size_t pixelsize(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM: return sizeof(uint32_t);
    case VK_FORMAT_R8_UNORM: return sizeof(uint8_t);
    case VK_FORMAT_R16_UNORM: return sizeof(uint16_t);
    }

    return 1;
}

size_t Image::size()
{
    size_t imgsize = width * height * pixelsize(format);
    size_t memsize = imgsize * (1 - pow(0.25f, mipmaps)) / (1 - 0.25f);

    return memsize;
}

void downsample_rgba(uint8_t* in, uint8_t* out, int width, int height)
{
    auto in_pixel = [in, width, height](int x, int y) -> uint8_t*
    {
        return in + (y * width + x) * 4;
    };

    auto out_pixel = [out, width = width / 2, height = height / 2](int x, int y) -> uint8_t*
    {
        return out + (y * width + x) * 4;
    };

    for (int y = 0; y < height / 2; y++)
    {
        for (int x = 0; x < width / 2; x++)
        {
            float r = in_pixel(x * 2, y * 2)[0] + in_pixel(x * 2 + 1, y * 2)[0] +
                in_pixel(x * 2, y * 2 + 1)[0] + in_pixel(x * 2 + 1, y * 2 + 1)[0];

            float g = in_pixel(x * 2, y * 2)[1] + in_pixel(x * 2 + 1, y * 2)[1] +
                in_pixel(x * 2, y * 2 + 1)[1] + in_pixel(x * 2 + 1, y * 2 + 1)[1];

            float b = in_pixel(x * 2, y * 2)[2] + in_pixel(x * 2 + 1, y * 2)[2] +
                in_pixel(x * 2, y * 2 + 1)[2] + in_pixel(x * 2 + 1, y * 2 + 1)[2];

            float a = in_pixel(x * 2, y * 2)[3] + in_pixel(x * 2 + 1, y * 2)[3] +
                in_pixel(x * 2, y * 2 + 1)[3] + in_pixel(x * 2 + 1, y * 2 + 1)[3];

            out_pixel(x, y)[0] = r * 0.25;
            out_pixel(x, y)[1] = g * 0.25;
            out_pixel(x, y)[2] = b * 0.25;
            out_pixel(x, y)[3] = a * 0.25;
        }
    }
}

template <class T>
void downsample(T* in, T* out, int width, int height)
{
    auto in_pixel = [in, width, height](int x, int y) -> T&
    {
        return *(in + (y * width + x) * 4);
    };

    auto out_pixel = [out, width = width / 2, height = height / 2](int x, int y) -> T&
    {
        return *(out + (y * width + x) * 4);
    };

    for (int y = 0; y < height / 2; y++)
    {
        for (int x = 0; x < width / 2; x++)
        {
            float pixel = in_pixel(x * 2, y * 2) + in_pixel(x * 2 + 1, y * 2) +
                          in_pixel(x * 2, y * 2 + 1) + in_pixel(x * 2 + 1, y * 2 + 1);

            out_pixel(x, y) = pixel * 0.25f;
        }
    }
}

template void downsample<uint8_t>(uint8_t* in, uint8_t* out, int width, int height);
template void downsample<uint16_t>(uint16_t* in, uint16_t* out, int width, int height);

void BuildMipmaps(uint8_t* data, uint32_t width, uint32_t height, VkFormat format, size_t mipmaps)
{
    size_t pixelsz = pixelsize(format);

    void (*downsample_func)(uint8_t* in, uint8_t* out, int width, int height);

    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM: downsample_func = downsample_rgba;
    case VK_FORMAT_R8_UNORM: downsample<uint8_t>;
    case VK_FORMAT_R16_UNORM: downsample<uint16_t>;
    }

    uint8_t* in = data;
    uint8_t* out;

    for (size_t i = 0; i < mipmaps - 1; i++)
    {
        size_t offset = width * height * pixelsz;
        out = in + offset;

        downsample_func(in, out, width, height);

        in = out;

        width /= 2;
        height /= 2;
    }
}

Image* LoadImage(const char* filename)
{
    const char* extension = strrchr(filename, '.');

    if (!extension)
    {
        std::cout << "Unknown image type: " << filename << std::endl;
        return nullptr;
    }

    extension++;

    if (strcmp(extension, "bmp") == 0) return LoadBMP(filename);
    if (strcmp(extension, "png") == 0) return LoadPNG(filename);

    std::cout << "Unknown image type: " << filename << std::endl;

    return nullptr;
}