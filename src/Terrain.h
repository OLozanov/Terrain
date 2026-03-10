#pragma once

#include "Render/Render.h"
#include "Render/Mesh.h"
#include "Grass.h"
#include "BBox.h"

#include "TerrainData.h"
#include "Cache.h"

#include "Sync.h"

#include <vector>
#include <map>
#include <deque>

struct GrassBuffer
{
    Render::VertexBuffer<GrassInstance> m_grass;
    size_t m_instanceNum;
};

struct Tile
{
    glm::mat4 mat;
    float lodDist;
};

struct GrassPatch
{
    BBox bbox;
    uint32_t id;
    uint32_t num;
    glm::vec2 pos;
};

class Terrain
{
private:
    using GrassCache = Cache<TileKey, GrassPatch>;

    static constexpr size_t GrassCacheSize = 100;

public:
    Terrain();

    const Image& heightmap() { return m_dataSource.heightmap(); }
    const Image& normals() { return m_dataSource.normals(); }

    uint32_t levels() const { return m_maxLevel; }

    float size() const { return m_size; }
    float height() const { return m_dataSource.heightScale(); }

private:
    void initGeometry();
    void initGrass();

    float lodDist(uint32_t level);
    float tileSize(uint32_t level);
    glm::vec2 tileOffset(const TileKey& tilekey);

    float grassPatchSize() { return m_grassPatchSize; }
    uint32_t grassDensity() { return m_grassDensity; }

    BBox getBBox(const TileKey& tilekey);
    void generateGrassPatch();
    void generateTile(const TileKey& tilekey);
    void generateTiles(const std::vector<TileKey>& tiles);
    void generateGrassPatch(GrassPatch& patch);
    void generateGrass(const std::vector<TileKey>& tiles);

    const Tile& tile(const TileKey& tilekey) const { return m_tiles.at(tilekey); }
    const GrassPatch& grasPatch(const TileKey& tilekey) const { return m_grassCache.getValue(tilekey); }

    VkBuffer tileIndexBuffer() const { return m_indexBuffer; }
    VkBuffer tileVertexBuffer() const { return m_vertexBuffer; }

private:
    TerrainData m_dataSource;

    Render::IndexBuffer m_indexBuffer;
    Render::VertexBuffer<glm::vec2> m_vertexBuffer;

    Render::Mesh<GrassVertex> m_grassBlade;

    float m_size;
    uint32_t m_maxLevel;

    float m_grassPatchSize;
    uint32_t m_grassDensity;

    std::vector<glm::vec4> m_grassParams;
    Render::Buffer m_grass;
    GrassInstance* m_grassData;
    GrassCache m_grassCache;

    std::map<TileKey, Tile> m_tiles;

    SpinLock m_dataLock;

    friend class TerrainView;
};

class TerrainView
{
public:
    TerrainView(Terrain& terrain, const Render::Camera& camera, const Render::Frustum& frustum)
    : m_terrain(terrain)
    , m_camera(camera)
    , m_frustum(frustum)
    {
    }

    void update();
    void updateGrass();

    void display(Render::CommandList& commandList) const;
    void displayGrass(Render::CommandList& commandList) const;
    void displayBBoxes(Render::CommandList& commandList) const;

private:
    void processTile(const TileKey& tileKey);

private:
    Terrain& m_terrain;

    const Render::Camera& m_camera;
    const Render::Frustum& m_frustum;

    std::deque<TileKey> m_processQueue;
    std::vector<TileKey> m_viewTiles;
    std::vector<TileKey> m_grassTiles;

    static constexpr float GrassLodDist = 4.0f;
};