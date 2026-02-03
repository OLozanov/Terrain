#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Terrain.h"

#include <algorithm>

Terrain::Terrain()
: m_size(64)
, m_maxLevel(4)
{
    initGeometry();

    m_dataSource.load("heightmaps/islands.png", 2.0);

    m_size = m_dataSource.size() / 2;
    m_maxLevel = m_dataSource.levels();

    float scale = m_size / (1 << m_maxLevel) / TileParams::GridSize;
}

void Terrain::initGeometry()
{
    std::vector<glm::vec2> vertices(TileParams::VertexNum);

    constexpr size_t stride = TileParams::GridSize + 1;

    for (int k = 0; k <= TileParams::GridSize; k++)
        for (int i = 0; i <= TileParams::GridSize; i++)
        {
            size_t idx = k * stride + i;
            vertices[idx] = { i - int(TileParams::GridSize / 2), k - int(TileParams::GridSize / 2) };
        }

    std::vector<uint16_t> indices(TileParams::IndexNum);

    size_t p = 0;

    for (uint16_t k = 0; k < TileParams::GridSize; k++)
        for (uint16_t i = 0; i < TileParams::GridSize; i++)
        {
            uint16_t v = i * stride + k;

            //
            indices[p++] = v;
            indices[p++] = v + 1;
            indices[p++] = v + stride;

            //
            indices[p++] = v + stride;
            indices[p++] = v + 1;
            indices[p++] = v + stride + 1;
        }

    m_vertexBuffer.setData(vertices.data(), vertices.size());
    m_indexBuffer.setData(indices.data(), indices.size());
}

float Terrain::tileSize(uint32_t level)
{
    uint32_t tnum = 1 << level;
    return m_size / tnum * 2.5f;
}

BBox Terrain::getBBox(const TileKey& tilekey)
{
    const HeightRange& range = m_dataSource.getTileRange(tilekey);

    if (tilekey.level == 0)
    {
        glm::vec3 bbox = { m_size * 0.5f, 0.0f, m_size * 0.5f };
        return { { -m_size * 0.5f, range.first, -m_size * 0.5f },
                 { m_size * 0.5f, range.second, m_size * 0.5f } };
    }

    uint32_t tnum = 1 << tilekey.level;
    
    float tilesz = m_size / tnum;
    float dim = tilesz * 0.5f;

    float x = tilesz * (int(tilekey.x) - int(tnum) / 2 + 0.5f);
    float y = tilesz * (int(tilekey.y) - int(tnum) / 2 + 0.5f);

    glm::vec3 pos = glm::vec3(x, 0, y);
    glm::vec3 min = { x - dim, range.first, y - dim };
    glm::vec3 max = { x + dim, range.second, y + dim };

    return { min, max };
}

void Terrain::generateTile(const TileKey& tilekey)
{
    if (m_tiles.find(tilekey) != m_tiles.end()) return;

    Tile& tile = m_tiles[tilekey];

    BBox bbox = getBBox(tilekey);

    glm::vec3 pos = (bbox.min + bbox.max) * 0.5f;
    pos.y = 0.0f;

    uint32_t tnum = 1 << tilekey.level;
    float tilesz = m_size / tnum;

    float scale = tilesz / TileParams::GridSize;

    glm::mat4 mat = glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f, scale));
    mat = glm::translate(glm::mat4(1.0f), pos) * mat;

    tile.mat = mat;
    tile.lodDist = tileSize(tilekey.level);
}

void Terrain::generateTiles(const std::vector<TileKey>& tiles)
{
    m_dataLock.lock();

    for (const TileKey& tile : tiles) generateTile(tile);

    m_dataLock.unlock();
}

void TerrainView::processTile(const TileKey& tilekey)
{
    BBox bbox = m_terrain.getBBox(tilekey);

    if (!m_frustum.test(bbox)) return;

    glm::vec3 pos = (bbox.min + bbox.max) * 0.5f;
    glm::vec3 extent = bbox.max - pos;

    uint32_t tnum = 1 << tilekey.level;
    float tilesz = m_terrain.size() / tnum;

    if (tilekey.level == m_terrain.levels())
    {
        m_viewTiles.push_back(tilekey);
        return;
    }

    float dist = m_terrain.tileSize(tilekey.level);

    if (bbox.intersectsSphere(m_camera.pos(), dist))
    {
        uint32_t childLevel = tilekey.level + 1;
        uint32_t x = tilekey.x * 2;
        uint32_t y = tilekey.y * 2;
    
        m_processQueue.push_back({ childLevel, x, y });
        m_processQueue.push_back({ childLevel, x + 1, y });
        m_processQueue.push_back({ childLevel, x, y + 1 });
        m_processQueue.push_back({ childLevel, x + 1, y + 1 });
    }
    else
    {
        m_viewTiles.push_back(tilekey);
    }
}

void TerrainView::update()
{
    m_viewTiles.clear();

    m_processQueue.push_back({ 0, 0, 0 });

    while (!m_processQueue.empty())
    {
        TileKey tilekey = m_processQueue.front();
        m_processQueue.pop_front();

        processTile(tilekey);
    }

    m_terrain.generateTiles(m_viewTiles);
}

void TerrainView::display(Render::CommandList& commandList) const
{
    commandList.bindIndexBuffer(m_terrain.tileIndexBuffer());
    commandList.bindVertexBuffer(m_terrain.tileVertexBuffer());

    commandList.setConstant(0, m_terrain.size());
    commandList.setConstant(4, m_terrain.height());

    for (const TileKey& tilekey : m_viewTiles)
    {
        const Tile& tile = m_terrain.tile(tilekey);

        commandList.setConstant(12, tile.lodDist);
        commandList.setConstant(16, tile.mat);
        commandList.drawIndexed(TileParams::IndexNum);
    }
}

void TerrainView::displayBBoxes(Render::CommandList& commandList) const
{
    for (const TileKey& tilekey : m_viewTiles)
    {
        BBox bbox = m_terrain.getBBox(tilekey);

        glm::vec3 pos = (bbox.min + bbox.max) * 0.5f;
        glm::vec3 extent = bbox.max - pos;

        glm::mat4 mat = glm::translate(glm::mat4(1.0), pos) * glm::scale(glm::mat4(1.0), extent);

        commandList.setConstant(0, mat);
        commandList.drawIndexed(24);
    }
}