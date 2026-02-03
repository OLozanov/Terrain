#include "Buffer.h"

#include <stdexcept>

namespace Render
{

Buffer::Buffer()
: m_buffer(VK_NULL_HANDLE)
{
}

Buffer::Buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memtype, size_t size)
: m_buffer(VK_NULL_HANDLE)
{
    reset(usage, memtype, size);
}

Buffer::~Buffer()
{
    reset();
}

void Buffer::allocateMemory(VkMemoryPropertyFlags flags)
{
    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkInstance.device(), m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vkInstance.detectMemoryType(memRequirements.memoryTypeBits, flags);

    if(vkAllocateMemory(vkInstance.device(), &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(vkInstance.device(), m_buffer, m_bufferMemory, 0);
}

void Buffer::reset()
{
    if (m_buffer != VK_NULL_HANDLE)
    {
        VulkanInstance& vkInstance = VulkanInstance::GetInstance();

        vkDestroyBuffer(vkInstance.device(), m_buffer, nullptr);
        vkFreeMemory(vkInstance.device(), m_bufferMemory, nullptr);
    }
}

void Buffer::reset(VkBufferUsageFlags usage, VkMemoryPropertyFlags memtype, size_t size)
{
    reset();

    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkInstance.device(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    allocateMemory(memtype);
}

void * Buffer::map(size_t size, size_t offset)
{
    void * mem;
    vkMapMemory(VulkanInstance::GetInstance().device(), m_bufferMemory, offset, size, 0, &mem);

    return mem;
}

void Buffer::unmap()
{
    vkUnmapMemory(VulkanInstance::GetInstance().device(), m_bufferMemory);
}

void IndexBuffer::setData(const uint16_t* data, size_t size)
{
    size_t bytes = size * sizeof(uint16_t);

    reset(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bytes);

    VulkanInstance::GetInstance().createBuffer(*this, data, bytes);
}

} // namespace Render