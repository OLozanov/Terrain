#pragma once

#include "Render/Render.h"
#include "BBox.h"

#include "TerrainData.h"

#include "Sync.h"

#include <vector>
#include <map>
#include <deque>

struct Tile
{
    glm::mat4 mat;
    float lodDist;
};

class Terrain
{
public:
    Terrain();

    const Image& heightmap() { return m_dataSource.heightmap(); }
    const Image& normals() { return m_dataSource.normals(); }

    uint32_t levels() const { return m_maxLevel; }

    float size() const { return m_size; }
    float height() const { return m_dataSource.height(); }

private:
    void initGeometry();

    float tileSize(uint32_t level);

    BBox getBBox(const TileKey& tilekey);
    void generateTile(const TileKey& tilekey);
    void generateTiles(const std::vector<TileKey>& tiles);

    const Tile& tile(const TileKey& tilekey) const { return m_tiles.at(tilekey); }

    VkBuffer tileIndexBuffer() const { return m_indexBuffer; }
    VkBuffer tileVertexBuffer() const { return m_vertexBuffer; }

private:
    TerrainData m_dataSource;

    Render::IndexBuffer m_indexBuffer;
    Render::VertexBuffer<glm::vec2> m_vertexBuffer;

    float m_size;
    uint32_t m_maxLevel;

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
    void display(Render::CommandList& commandList) const;
    void displayBBoxes(Render::CommandList& commandList) const;

private:
    void processTile(const TileKey& tileKey);

private:
    Terrain& m_terrain;

    const Render::Camera& m_camera;
    const Render::Frustum& m_frustum;

    std::deque<TileKey> m_processQueue;
    std::vector<TileKey> m_viewTiles;
};