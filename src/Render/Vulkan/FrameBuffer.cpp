#include "FrameBuffer.h"
#include "Render/Vulkan/VulkanInstance.h"
#include "Render/Vulkan/Bitmap.h"

#include <stdexcept>
#include <iostream>

namespace Render
{

FrameBuffer::FrameBuffer()
{
    m_renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    m_renderInfo.renderArea = {};
    m_renderInfo.layerCount = 1;
    m_renderInfo.colorAttachmentCount = 1;
    m_renderInfo.pColorAttachments = &m_colorAttachmentInfo;
    m_renderInfo.pDepthAttachment = nullptr;
    m_renderInfo.pStencilAttachment = nullptr;
}

void FrameBuffer::resize(const VkExtent2D& frameExtent)
{
    m_renderInfo.renderArea = { {0, 0}, frameExtent };
}

void FrameBuffer::addColorAttachment(Bitmap& color)
{
    m_colorBuffer = &color;

    m_colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    m_colorAttachmentInfo.imageView = color;
    m_colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    m_colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_colorAttachmentInfo.clearValue = { 0, 0 };

    m_renderInfo.layerCount = 1;
    m_renderInfo.colorAttachmentCount = 1;
    m_renderInfo.pColorAttachments = &m_colorAttachmentInfo;
}

void FrameBuffer::addDepthAttachment(Bitmap& depth)
{
    m_depthBuffer = &depth;

    m_depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    m_depthAttachmentInfo.imageView = depth;
    m_depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    m_depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_depthAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

    m_renderInfo.pDepthAttachment = &m_depthAttachmentInfo;
    m_renderInfo.pStencilAttachment = &m_depthAttachmentInfo;
}

void FrameBuffer::setClearColor(float r, float g, float b)
{
    m_colorAttachmentInfo.clearValue.color.float32[0] = r;
    m_colorAttachmentInfo.clearValue.color.float32[1] = g;
    m_colorAttachmentInfo.clearValue.color.float32[2] = b;
}

} // namespace Render
