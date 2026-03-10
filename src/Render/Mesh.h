#pragma once

#include "Render/Render.h"
#include <vector>

namespace Render
{

template <class T>
class Mesh
{
public:
    Mesh() : m_vnum(0) {}

    bool empty() const { return m_vnum == 0; }

    size_t size() const { return m_vnum; }

    void setData(const std::vector<T>& vertices, const std::vector<uint16_t>& indices)
    {
        m_vertexBuffer.setData(vertices.data(), vertices.size());
        m_indexBuffer.setData(indices.data(), indices.size());

        m_vnum = indices.size();
    }

    void display(CommandList& commandList) const
    {
        commandList.bindIndexBuffer(m_indexBuffer);
        commandList.bindVertexBuffer(m_vertexBuffer);
        commandList.drawIndexed(m_vnum);
    }

    VkBuffer indexBuffer() const { return m_indexBuffer; }
    VkBuffer vertexBuffer() const { return m_vertexBuffer; }

private:
    size_t m_vnum;

    Render::IndexBuffer m_indexBuffer;
    Render::VertexBuffer<T> m_vertexBuffer;
};

} // namespace Render