#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Terrain.h"

#include <random>
#include <algorithm>

Terrain::Terrain()
: m_size(64)
, m_maxLevel(4)
, m_grassCache(GrassCacheSize)
{
    initGeometry();
    initGrass();

    generateGrassPatch();

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

void Terrain::initGrass()
{
    std::vector<GrassVertex> verts = { // Front
                                     {{-0.043f, 0.0f}, {-0.25f, 1.0f}}, {{0.043f, 0.0f}, {0.25f, 1.0f}},
                                     {{-0.04f, 0.2f}, {-0.25f, 1.0f}},  {{0.04f, 0.2f}, {0.25f, 1.0f}},
                                     {{-0.035f, 0.4f}, {-0.25f, 1.0f}}, {{0.035f, 0.4f}, {0.25f, 1.0f}},
                                     {{-0.03f, 0.6f}, {-0.25f, 1.0f}},  {{0.03f, 0.6f}, {0.25f, 1.0f}},
                                     {{-0.02f, 0.8f}, {-0.25f, 1.0f}},  {{0.02f, 0.8f}, {0.25f, 1.0f}},
                                     {{0.0f, 1.0f}, {0.0f, 1.0f}} };
                                     // Back
                                     /*{{-0.043f, 0.0f}, {-0.25f, -1.0f}}, {{0.043f, 0.0f}, {0.25f, -1.0f}},
                                     {{-0.04f, 0.2f}, {-0.25f, -1.0f}},  {{0.04f, 0.2f}, {0.25f, -1.0f}},
                                     {{-0.035f, 0.4f}, {-0.25f, -1.0f}}, {{0.035f, 0.4f}, {0.25f, -1.0f}},
                                     {{-0.03f, 0.6f}, {-0.25f, -1.0f}},  {{0.03f, 0.6f}, {0.25f, -1.0f}},
                                     {{-0.02f, 0.8f}, {-0.25f, -1.0f}},  {{0.02f, 0.8f}, {0.25f, -1.0f}},
                                     {{0.0f, 1.0f}, {0.0f, -1.0f}} };*/

    std::vector<uint16_t> indices = { // High LOD
                                      2, 0, 1, 1, 3, 2,
                                      4, 2, 3, 3, 5, 4,
                                      6, 4, 5, 5, 7, 6,
                                      8, 6, 7, 7, 9, 8,
                                      10, 8, 9,
                                      // Low LOD
                                      10, 0, 1
                                      };

    m_grassBlade.setData(verts, indices);
}

float Terrain::lodDist(uint32_t level)
{
    uint32_t tnum = 1 << level;
    return m_size / tnum * 2.5f;
}

float Terrain::tileSize(uint32_t level)
{
    uint32_t tnum = 1 << level;
    return m_size / tnum;
}

glm::vec2 Terrain::tileOffset(const TileKey& tilekey)
{
    uint32_t tnum = 1 << tilekey.level;

    float tilesz = m_size / tnum;

    float x = tilesz * (int(tilekey.x) - int(tnum) / 2 + 0.5f);
    float y = tilesz * (int(tilekey.y) - int(tnum) / 2 + 0.5f);

    return glm::vec2(x, y);
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

void Terrain::generateGrassPatch()
{
    std::random_device rd;
    std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
    std::mt19937 uniformGenerator(seq);

    std::uniform_real_distribution<float> rotDistribution(0.0f, 2.0 * glm::pi<float>());
    std::uniform_real_distribution<float> offsetDistribution(-0.25f, 0.25f);

    constexpr float density = 10.0f;
    float sz = tileSize(m_maxLevel) * 2.0f;

    size_t n = TileParams::GridSize * density;
    float step = sz / n;

    m_grassPatchSize = sz;
    m_grassDensity = n;

    for (size_t k = 0; k < n; k++)
        for (size_t i = 0; i < n; i++)
        {
            float xoffset = offsetDistribution(uniformGenerator);
            float yoffset = offsetDistribution(uniformGenerator);

            float x = (i + 0.5f + xoffset) * step - sz * 0.5f;
            float y = (k + 0.5f + yoffset) * step - sz * 0.5f;

            glm::vec3 pos = {x, 0.0f, y};
            float rot = rotDistribution(uniformGenerator);

            float rcos = cosf(rot);
            float rsin = sinf(rot);

            m_grassParams.emplace_back(x, y, rcos, rsin);
        }

    size_t bytes = n * n * sizeof(GrassInstance) * GrassCacheSize;
    m_grass.reset(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                  bytes);

    m_grassData = static_cast<GrassInstance*>(m_grass.map(bytes));
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
    tile.lodDist = lodDist(tilekey.level);
}

void Terrain::generateTiles(const std::vector<TileKey>& tiles)
{
    m_dataLock.lock();

    for (const TileKey& tile : tiles) generateTile(tile);

    m_dataLock.unlock();
}

void Terrain::generateGrassPatch(GrassPatch& patch)
{
    size_t n = m_grassDensity * m_grassDensity;
    size_t offset = n * patch.id;

    std::vector<GrassInstance> grass;
    grass.reserve(n);

    for (size_t i = 0; i < n; i++)
    {
        const glm::vec4& params = m_grassParams[i];

        glm::vec2 tpos = glm::vec2((patch.pos.x + params.x) / (m_size - 0.5f) + 0.5f, 
                                   (patch.pos.y + params.y) / (m_size - 0.5f) + 0.5f);

        float h = m_dataSource.height(tpos.x, tpos.y);

        if (h > 12.0f && h < 31.0f)
        {
            glm::vec3 pos = { params.x, h, params.y };
            grass.push_back({ pos, {params.z, params.w} });
        }
    }

    patch.num = grass.size();

    if (patch.num > 0)
        memcpy(m_grassData + offset, grass.data(), grass.size() * sizeof(GrassInstance));
}

void Terrain::generateGrass(const std::vector<TileKey>& tiles)
{
    std::vector<TileKey> newTiles;

    for (const TileKey& tile : tiles)
    {
        if (tile.level != m_maxLevel) continue;
        
        size_t n = m_grassCache.touchNode(tile);

        if (n == GrassCache::InvalidNode) newTiles.push_back(tile);
    }

    for (const TileKey& tile : newTiles)
    {
        size_t n = m_grassCache.addNode(tile);

        GrassPatch& patch = m_grassCache[n];

        patch.bbox = getBBox(tile);
        patch.pos = tileOffset(tile);
        patch.id = n;

        generateGrassPatch(patch);
    }
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

    float dist = m_terrain.lodDist(tilekey.level);

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

void TerrainView::updateGrass()
{
    m_grassTiles.clear();

    for (const TileKey& tile : m_viewTiles)
    {
        if (tile.level == m_terrain.levels()) m_grassTiles.push_back(tile);
    }

    m_terrain.generateGrass(m_grassTiles);
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

void TerrainView::displayGrass(Render::CommandList& commandList) const
{
    commandList.bindIndexBuffer(m_terrain.m_grassBlade.indexBuffer());
    commandList.bindVertexBuffer(m_terrain.m_grassBlade.vertexBuffer());
    commandList.bindVertexBuffer(1, m_terrain.m_grass);

    size_t stride = m_terrain.m_grassDensity * m_terrain.m_grassDensity;

    for (const TileKey& tile: m_grassTiles)
    {
        const GrassPatch& patch = m_terrain.grasPatch(tile);

        if (patch.num == 0) continue;

        bool highlod = patch.bbox.intersectsSphere(m_camera.pos(), GrassLodDist);

        uint32_t vnum = highlod ? 27 : 3;
        uint32_t offset = highlod ? 0 : 27;

        commandList.setConstant(0, patch.pos);
        commandList.drawIndexed(vnum, patch.num, offset, 0, patch.id * stride);
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