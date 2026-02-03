#pragma once

#include "Render/Render.h"

class SkyDome
{
public:
    SkyDome(int segments, int rings, float radius, float angle);

    void display(Render::CommandList& commandList) const;

private:
    size_t m_vnum;

    Render::IndexBuffer m_indexBuffer;
    Render::VertexBuffer<glm::vec3> m_vertexBuffer;
};