#pragma once

#include <vulkan/vulkan.h>

namespace Render
{

class Bitmap;

class FrameBuffer
{
    VkRenderingAttachmentInfo m_colorAttachmentInfo = {};
    VkRenderingAttachmentInfo m_depthAttachmentInfo = {};
    VkRenderingInfo m_renderInfo = {};

    Bitmap* m_colorBuffer;
    Bitmap* m_depthBuffer; 

public:
    FrameBuffer();
    ~FrameBuffer() = default;

    void resize(const VkExtent2D& frameExtent);

    void addColorAttachment(Bitmap& color);
    void addDepthAttachment(Bitmap& depth);

    void setClearColor(float r, float g, float b);

    operator const VkRenderingInfo&() const { return m_renderInfo; }

    friend class CommandList;
};

} // namespace Render
