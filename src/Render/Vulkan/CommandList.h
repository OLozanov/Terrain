#pragma once

#include <vulkan/vulkan.h>

#include "Render/Vulkan/Pipeline.h"

namespace Render
{

class Bitmap;
class FrameBuffer;

class CommandList
{
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    VkPipelineLayout m_layout;

    bool m_clear;
    VkClearColorValue m_clearColor = {};

public:

    CommandList();
    CommandList(VkCommandBuffer commandBuffer);
    ~CommandList();

    void begin();
    void finish();

    void setViewport(uint32_t width, uint32_t height);

    void clearColor(float r, float g, float b, float a = 1.0);
    
    void bindFrameBuffer(const VkRenderingInfo& renderInfo);
    void bindFrameBuffer(const VkExtent2D& frameExtent, const VkImageView& colorBuffer);
    void bindFrameBuffer(const VkExtent2D& frameExtent, const VkImageView& colorBuffer, const VkImageView& depthBuffer);
    void finishRender();

    void bindPipeline(const Pipeline& graphicsPipeline);
    void bindIndexBuffer(VkBuffer buffer);
    void bindVertexBuffer(VkBuffer buffer);
    void bindDescriptorSet(VkDescriptorSet descriptorSet);

    void setPolygonMode(VkPolygonMode mode);
    void setCullMode(VkCullModeFlags mode);

    void bind(uint32_t binding, VkBuffer buffer, VkDeviceSize size);
    void bind(uint32_t binding, VkImageView image, VkSampler sampler);
    void bind(uint32_t binding, VkImageView image);

    template<class T>
    void setConstant(uint32_t offset, const T& value, VkShaderStageFlags flags = VK_SHADER_STAGE_VERTEX_BIT)
    {
        vkCmdPushConstants(m_commandBuffer, m_layout, flags, offset, sizeof(T), &value);
    }

    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void drawIndexed(uint16_t num);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, size_t mipmaps);

    void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

    void blitImage(VkImage src,
                   VkImage dst,
                   const VkOffset3D& srcOffsetMin,
                   const VkOffset3D& srcOffsetMax,
                   const VkOffset3D& dstOffsetMin,
                   const VkOffset3D& dstOffsetMax,
                   VkFilter filter);

    void barrier(const VkImageMemoryBarrier& barrier,
                       VkPipelineStageFlags sourceStage,
                       VkPipelineStageFlags destinationStage);

    void barrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void barrier(Bitmap& bitmap, VkImageLayout layout);

    void barrier(FrameBuffer& framebuffer, VkImageLayout layout);

    void reset();

    operator VkCommandBuffer() const { return m_commandBuffer; }

private:
    VkCommandBuffer* operator&() { return &m_commandBuffer; }

    VkCommandBuffer operator=(VkCommandBuffer commandBuffer)
    {
        m_commandBuffer = commandBuffer;
        return m_commandBuffer;
    }

    static void LoadExtFunctions(VkDevice& device);

    friend class VulkanInstance;
};

} // namespace Render
