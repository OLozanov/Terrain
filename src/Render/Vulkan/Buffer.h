#pragma once

#include <vulkan/vulkan.h>

#include "Render/Vulkan/VulkanInstance.h"

namespace Render
{

class Buffer
{
    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;

    void allocateMemory(VkMemoryPropertyFlags flags);

public:

    Buffer();
    Buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memtype, size_t size);
    ~Buffer();

    void reset();
    void reset(VkBufferUsageFlags usage, VkMemoryPropertyFlags memtype, size_t size);

    void * map(size_t size, size_t offset = 0);
    void unmap();

    operator VkBuffer() const { return m_buffer; }
};

class IndexBuffer : public Buffer
{
public:

    IndexBuffer() {}

    void setData(const uint16_t* data, size_t size);
};

template<class T>
class VertexBuffer : public Buffer
{
public:
    VertexBuffer() {}

    void setData(const T* data, size_t size)
    {
        size_t bytes = size * sizeof(T);

        reset(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bytes);

        VulkanInstance::GetInstance().createBuffer(*this, data, bytes);
    }
};

} // namespace Render