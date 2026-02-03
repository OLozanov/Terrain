#include "SkyDome.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

SkyDome::SkyDome(int segments, int rings, float radius, float angle)
{
    std::vector<glm::vec3> vertices;
    std::vector<uint16_t> indices;

    vertices.reserve(1 + segments * rings);
    indices.reserve(segments * 3 + (rings - 1) * segments * 6);

    vertices.push_back(glm::vec3(0));

    float dang = glm::pi<float>() * 2.0f / segments;
    float dang_vert = (angle / 180.0f * glm::pi<float>()) / rings;

    for (int k = 0; k < rings; k++)
    {
        float vang = dang_vert * k;

        float h = radius * (1 - cosf(vang));
        float r = radius * sinf(vang);

        for (int i = 0; i < segments; i++)
        {
            float ang = dang * i;

            float x = r * cosf(ang);
            float z = r * sinf(ang);
            float y = -h;

            vertices.emplace_back(x, y, z);
        }
    }

    // Central ring
    for (int i = 0; i < segments; i++)
    {
        uint16_t a = i + 1;
        uint16_t b = a == segments ? 1 : a + 1;

        indices.push_back(0);
        indices.push_back(b);
        indices.push_back(a);
    }

    // Outer rings
    for (int k = 0; k < rings - 1; k++)
    {
        uint16_t r1 = 1 + k * segments;
        uint16_t r2 = r1 + segments;

        for (int i = 0; i < segments; i++)
        {
            uint16_t a = r1 + i;
            uint16_t b = (i + 1) == segments ? r1 : a + 1;

            uint16_t c = r2 + i;
            uint16_t d = (i + 1) == segments ? r2 : c + 1;

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(d);

            indices.push_back(d);
            indices.push_back(c);
            indices.push_back(a);
        }
    }

    m_vertexBuffer.setData(vertices.data(), vertices.size());
    m_indexBuffer.setData(indices.data(), indices.size());

    m_vnum = indices.size();
}

void SkyDome::display(Render::CommandList& commandList) const
{
    commandList.bindIndexBuffer(m_indexBuffer);
    commandList.bindVertexBuffer(m_vertexBuffer);
    commandList.drawIndexed(m_vnum);
}