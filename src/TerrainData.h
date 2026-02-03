#pragma once

#include "Resources/Image.h"

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <utility>
#include <memory>

struct TileParams
{
    TileParams() = delete;
    ~TileParams() = delete;

    static constexpr size_t GridSize = 16;
    static constexpr size_t VertexNum = (GridSize + 1) * (GridSize + 1);
    static constexpr size_t IndexNum = GridSize * GridSize * 6;
};

struct TileKey
{
    uint32_t level;
    uint32_t x;
    uint32_t y;

    bool operator<(const TileKey& key) const
    {
        if (level != key.level) return level < key.level;
        if (x != key.x) return x < key.x;
        return y < key.y;
    }
};

using HeightRange = std::pair<float, float>;

class TerrainData
{
public:
    void generateData(uint32_t size, float scale);

    void load(const char* filename, float scale);

    float height() const { return m_height; }
    uint32_t size() const { return m_size; }
    uint32_t levels() const { return m_levels; }

    const HeightRange& getTileRange(const TileKey& tilekey) const;

    const Image& heightmap() const { return *m_heightmap; }
    const Image& normals() const { return *m_normals; }

private:
    void buildNormals();
    void buildLayers();

    void generateTile(uint32_t level, uint32_t x, uint32_t y);
    void generateLevelTiles(uint32_t level);
    void generateTiles();

private:
    uint32_t m_size;
    uint32_t m_levels;
    uint32_t m_scale;
    
    std::vector<float> m_data;

    float m_height = 150.0f;

    std::unique_ptr<Image> m_heightmap;
    std::unique_ptr<Image> m_normals;
    std::unique_ptr<Image> m_layermap;

    std::map<TileKey, HeightRange> m_ranges;
};