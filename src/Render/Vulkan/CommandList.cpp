#include "CommandList.h"
#include "Render/Vulkan/VulkanInstance.h"
#include "Render/Vulkan/Bitmap.h"
#include "Render/Vulkan/FrameBuffer.h"

#include <stdexcept>

namespace Render
{

PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonMode;

void CommandList::LoadExtFunctions(VkDevice& device)
{
    vkCmdSetPolygonMode = (PFN_vkCmdSetPolygonModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT");
}

CommandList::CommandList()
{
    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    vkInstance.createCommandPool(&m_commandPool);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(vkInstance.device(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

CommandList::CommandList(VkCommandBuffer commandBuffer)
: m_commandBuffer(commandBuffer)
{
}

CommandList::~CommandList()
{
    if (!m_commandBuffer) return;

    VulkanInstance& vkInstance = VulkanInstance::GetInstance();

    vkFreeCommandBuffers(vkInstance.device(), m_commandPool, 1, &m_commandBuffer);
    vkDestroyCommandPool(vkInstance.device(), m_commandPool, nullptr);
}

void CommandList::begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    m_clear = false;
}

void CommandList::finish()
{
    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandList::setViewport(uint32_t width, uint32_t height)
{
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (height);
    viewport.width = float(width);
    viewport.height = -float(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { width, height };

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void CommandList::clearColor(float r, float g, float b, float a)
{
    m_clearColor.float32[0] = r;
    m_clearColor.float32[1] = g;
    m_clearColor.float32[2] = b;
    m_clearColor.float32[3] = a;

    m_clear = true;
}

void CommandList::bindFrameBuffer(const VkRenderingInfo& renderInfo)
{
    vkCmdBeginRendering(m_commandBuffer, &renderInfo);
}

void CommandList::bindFrameBuffer(const VkExtent2D& frameExtent, const VkImageView& colorBuffer)
{
    VkRenderingAttachmentInfo colorAttachmentInfo = {};
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachmentInfo.imageView = colorBuffer;
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.loadOp = m_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentInfo.clearValue.color = m_clearColor;

    VkRenderingInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea = { {0, 0}, frameExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachmentInfo;
    renderInfo.pDepthAttachment = nullptr;
    renderInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(m_commandBuffer, &renderInfo);
}

void CommandList::bindFrameBuffer(const VkExtent2D& frameExtent, const VkImageView& colorBuffer, const VkImageView& depthBuffer)
{
    VkRenderingAttachmentInfo colorAttachmentInfo = {};
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachmentInfo.imageView = colorBuffer;
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.loadOp = m_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentInfo.clearValue.color = m_clearColor;

    VkRenderingAttachmentInfo depthAttachmentInfo = {};
    depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachmentInfo.imageView = depthBuffer;
    depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

    VkRenderingInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea = { {0, 0}, frameExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachmentInfo;
    renderInfo.pDepthAttachment = &depthAttachmentInfo;
    renderInfo.pStencilAttachment = &depthAttachmentInfo;

    vkCmdBeginRendering(m_commandBuffer, &renderInfo);
}

void CommandList::finishRender()
{
    vkCmdEndRendering(m_commandBuffer);
    m_clear = false;
}

void CommandList::bindPipeline(const Pipeline& graphicsPipeline)
{
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    m_layout = graphicsPipeline.pipelineLayout();
}

void CommandList::bindIndexBuffer(VkBuffer buffer)
{
    vkCmdBindIndexBuffer(m_commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
}

void CommandList::bindVertexBuffer(VkBuffer buffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &buffer, &offset);
}

void CommandList::bindDescriptorSet(VkDescriptorSet descriptorSet)
{
    vkCmdBindDescriptorSets(m_commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_layout,
                            0, 1,
                            &descriptorSet,
                            0, nullptr);
}

void CommandList::setPolygonMode(VkPolygonMode mode)
{
    vkCmdSetPolygonMode(m_commandBuffer, mode);
}

void CommandList::setCullMode(VkCullModeFlags mode)
{
    vkCmdSetCullMode(m_commandBuffer, mode);
}

void CommandList::bind(uint32_t binding, VkBuffer buffer, VkDeviceSize size)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet writeInfo = {  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                        .pNext = nullptr,
                                        .dstSet = 0,
                                        .dstBinding = binding,
                                        .dstArrayElement = 0,
                                        .descriptorCount = 1,
                                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        .pImageInfo = nullptr,
                                        .pBufferInfo = &bufferInfo,
                                        .pTexelBufferView = nullptr,
                                     };

    vkCmdPushDescriptorSet(m_commandBuffer,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           m_layout,
                           0, 1,
                           &writeInfo);
}

void CommandList::bind(uint32_t binding, VkImageView image, VkSampler sampler)
{
    VkDescriptorImageInfo imageInfo = { .sampler = sampler,
                                        .imageView = image,
                                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      };

    VkWriteDescriptorSet writeInfo = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                       .dstSet = 0,
                                       .dstBinding = binding,
                                       .descriptorCount = 1,
                                       .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                       .pImageInfo = &imageInfo
                                      };

    vkCmdPushDescriptorSet(m_commandBuffer, 
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           m_layout, 
                           0, 1,
                           &writeInfo);
}

void CommandList::bind(uint32_t binding, VkImageView image)
{
    VkDescriptorImageInfo imageInfo = { .sampler = VK_NULL_HANDLE,
                                        .imageView = image,
                                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet writeInfo = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                       .dstSet = 0,
                                       .dstBinding = binding,
                                       .descriptorCount = 1,
                                       .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                       .pImageInfo = &imageInfo
    };

    vkCmdPushDescriptorSet(m_commandBuffer,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           m_layout,
                           0, 1,
                           &writeInfo);
}

void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandList::drawIndexed(uint16_t num)
{
    vkCmdDrawIndexed(m_commandBuffer, num, 1, 0, 0, 0);
}

void CommandList::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size)
{
    VkBufferCopy copyRegion {};
    copyRegion.size = size;

    vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void CommandList::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkBufferImageCopy copyRegion {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;

    copyRegion.imageOffset = {0, 0, 0};
    copyRegion.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(m_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
}

void CommandList::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, size_t mipmaps)
{
    std::vector<VkBufferImageCopy> copyRegion(mipmaps);

    size_t offset = 0;

    for (size_t i = 0; i < mipmaps; i++)
    {
        copyRegion[i].bufferOffset = offset;
        copyRegion[i].bufferRowLength = 0;
        copyRegion[i].bufferImageHeight = 0;

        copyRegion[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion[i].imageSubresource.mipLevel = i;
        copyRegion[i].imageSubresource.baseArrayLayer = 0;
        copyRegion[i].imageSubresource.layerCount = 1;

        copyRegion[i].imageOffset = { 0, 0, 0 };
        copyRegion[i].imageExtent = { width, height, 1 };

        offset += width * height * sizeof(uint32_t);

        width /= 2;
        height /= 2;
    }

    vkCmdCopyBufferToImage(m_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipmaps, copyRegion.data());
}

void CommandList::copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
{
    VkImageCopy copyRegion = {};
    copyRegion.extent.width = width;
    copyRegion.extent.height = height;
    copyRegion.extent.depth = 1;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdCopyImage(m_commandBuffer,
                   srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &copyRegion);
}

void CommandList::blitImage(VkImage src,
                            VkImage dst,
                            const VkOffset3D& srcOffsetMin,
                            const VkOffset3D& srcOffsetMax,
                            const VkOffset3D& dstOffsetMin,
                            const VkOffset3D& dstOffsetMax,
                            VkFilter filter)
{
    VkImageBlit blitRegion {};
    blitRegion.srcOffsets[0] = srcOffsetMin;
    blitRegion.srcOffsets[1] = srcOffsetMax;
    blitRegion.dstOffsets[0] = dstOffsetMin;
    blitRegion.dstOffsets[1] = dstOffsetMax;

    vkCmdBlitImage(m_commandBuffer,
                    src, VK_IMAGE_LAYOUT_UNDEFINED,
                    dst, VK_IMAGE_LAYOUT_UNDEFINED,
                    1, &blitRegion,
                    filter);
}

void CommandList::reset()
{
    vkResetCommandBuffer(m_commandBuffer, 0);
}

void CommandList::barrier(const VkImageMemoryBarrier& barrier,
                          VkPipelineStageFlags sourceStage,
                          VkPipelineStageFlags destinationStage)
{
    vkCmdPipelineBarrier(
    m_commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier);
}

void CommandList::barrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStageMask = 0;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    
    if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    
    if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    vkCmdPipelineBarrier(m_commandBuffer,
                         srcStageMask, dstStageMask,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}

void CommandList::barrier(Bitmap& bitmap, VkImageLayout layout)
{
    VkImageLayout oldLayout = bitmap.transitLayout(layout);
    if (layout != oldLayout) barrier(bitmap, oldLayout, layout);
}

void CommandList::barrier(FrameBuffer& framebuffer, VkImageLayout layout)
{
    barrier(*framebuffer.m_colorBuffer, layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL ? 
                                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : 
                                                  layout);

    barrier(*framebuffer.m_depthBuffer, layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL ?
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                                                  layout);
}

} // namespace Render
