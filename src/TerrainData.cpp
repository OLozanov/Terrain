#include "TerrainData.h"
#include "Render/Render.h"

#include <random>
#include <numeric>

constexpr float ipow(float a, int p)
{
    if (p == 2) return a * a;

    return ipow(a, p - 1);
}

float ease(float t)
{
    return 6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t;
}

void TerrainData::generateData(uint32_t size, float scale)
{
    uint32_t width = m_heightmap->width;
    uint32_t height = m_heightmap->height;

    m_heightmap = std::make_unique<Image>();
    m_heightmap->format = VK_FORMAT_R8G8B8A8_UNORM;
    m_heightmap->width = width;
    m_heightmap->height = height;
    m_heightmap->mipmaps = 1;
    size_t dataSize = width * height * sizeof(uint32_t);

    Render::Buffer buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
    uint32_t* heightmap = reinterpret_cast<uint32_t*>(buffer.map(dataSize));

    m_size = size;
    m_levels = uint32_t(log2f(m_size)) - log2f(TileParams::GridSize);
    m_scale = scale;

    m_data.resize((size + 1) * (size + 1));

    uint32_t gridSize = size >> 4;
    uint32_t stride = gridSize + 1;

    std::vector<float> grid((gridSize + 1) * (gridSize + 1));

    std::mt19937 uniformGenerator;
    uniformGenerator.seed(1729);

    std::uniform_real_distribution<float> distribution(0, 2.0);

    for (auto& val : grid) val = distribution(uniformGenerator);

    auto value = [&](float x, float y) -> float
    {
        uint32_t i = floor(x);
        uint32_t k = floor(y);

        if (i + 1 > gridSize) i = gridSize - 1;
        if (k + 1 > gridSize) k = gridSize - 1;

        float u = ease(x - floor(x));
        float v = ease(y - floor(y));

        float a = grid[k * stride + i];
        float b = grid[k * stride + i + 1];
        float c = grid[(k + 1) * stride + i];
        float d = grid[(k + 1) * stride + i + 1];

        float x1 = a * (1.0f - u) + b * u;
        float x2 = c * (1.0f - u) + d * u;

        return x1 * (1.0f - v) + x2 * v;
    };

    for (uint32_t k = 0; k <= size; k++)
        for (uint32_t i = 0; i <= size; i++)
        {
            uint32_t ind = k * (size + 1) + i;

            heightmap[ind] = value(i / 16.0f, k / 16.0f);
        }

    buffer.unmap();

    Render::VulkanInstance::GetInstance().createTexture(buffer, *m_heightmap);

    buildNormals();
    buildLayers();
    generateTiles();
}

void TerrainData::buildNormals()
{
    uint32_t width = m_heightmap->width;
    uint32_t height = m_heightmap->height;

    m_normals = std::make_unique<Image>();
    m_normals->format = VK_FORMAT_R8G8B8A8_UNORM;
    m_normals->width = width;
    m_normals->height = height;
    m_normals->mipmaps = 1;
    size_t size = width * height * sizeof(uint32_t);

    Render::Buffer buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
    uint32_t* normals = reinterpret_cast<uint32_t*>(buffer.map(size));

    auto heightmap = [data = reinterpret_cast<uint16_t*>(m_heightmap->data), width, scale = m_height * m_scale](size_t x, size_t y) -> float
    {
        uint16_t val = *(data + (y * width + x));
        return val / 65535.0f * scale;
    };

    auto normal = [normals, width](size_t x, size_t y) -> uint32_t&
    {
        return *(normals + (y * width + x));
    };

    for (size_t k = 0; k < height; k++)
    {
        for (size_t i = 0; i < width; i++)
        {
            float p = heightmap(i, k);
            float px = (i + 1) == width ? heightmap(i - 1, k) : heightmap(i + 1, k);
            float py = (k + 1) == height ? heightmap(i, k - 1) : heightmap(i, k + 1);

            float dx = (i + 1) == width ? p - px : px - p;  // 1, 0, dx
            float dy = (k + 1) == height ? p - py : py - p; // 0, 1, dy

            float x = -dx;
            float y = dy;
            float z = 1.0f;

            float len = sqrt(x * x + y * y + z * z);

            x /= len;
            y /= len;
            z /= len;

            uint32_t r = (x * 0.5f + 0.5f) * 255.0f;
            uint32_t g = (y * 0.5f + 0.5f) * 255.0f;
            uint32_t b = (z * 0.5f + 0.5f) * 255.0f;

            uint32_t color = r | g << 8 | b << 16;

            normal(i, k) = color;
        }
    }

    buffer.unmap();

    Render::VulkanInstance::GetInstance().createTexture(buffer, *m_normals);
}

void TerrainData::buildLayers()
{
    uint32_t width = m_heightmap->width;
    uint32_t height = m_heightmap->height;

    m_layermap = std::make_unique<Image>();
    m_layermap->format = VK_FORMAT_R8_UINT;
    m_layermap->width = width;
    m_layermap->height = height;
    m_layermap->mipmaps = 1;
    size_t size = width * height * sizeof(uint8_t);

    Render::Buffer buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size);
    uint8_t* layers = reinterpret_cast<uint8_t*>(buffer.map(size));

    auto heightmap = [data = reinterpret_cast<uint16_t*>(m_heightmap->data), width, scale = m_height](size_t x, size_t y) -> float
    {
        uint16_t val = *(data + (y * width + x));
        return val / 65535.0f * scale;
    };

    auto layerid = [layers, width](size_t x, size_t y) -> uint8_t&
    {
        return *(layers + (y * width + x));
    };

    for (size_t k = 0; k < height; k++)
    {
        for (size_t i = 0; i < width; i++)
        {
            float h = heightmap(i, k);
            
            uint8_t layer = 0; // grass

            if (h < 10.0f) layer = 1; // sand
            if (h > 30.0f) layer = 2; // rock

            layerid(i, k) = layer;
        }
    }

    buffer.unmap();

    Render::VulkanInstance::GetInstance().createTexture(buffer, *m_layermap);
}

void TerrainData::load(const char* filename, float scale)
{
    m_heightmap.reset(LoadPNG(filename, false, true));

    assert(m_heightmap->width == m_heightmap->height);

    m_size = m_heightmap->width;
    m_levels = uint32_t(log2f(m_size)) - log2f(TileParams::GridSize);
    m_scale = scale;

    buildNormals();
    buildLayers();
    generateTiles();
}

void TerrainData::generateTile(uint32_t level, uint32_t x, uint32_t y)
{
    HeightRange& range = m_ranges[{level, x, y}];

    range = { std::numeric_limits<float>::infinity(), 
              -std::numeric_limits<float>::infinity() };

    const uint32_t step = 1 << (m_levels - level);

    for (uint32_t k = 0; k <= TileParams::GridSize; k++)
        for (uint32_t i = 0; i <= TileParams::GridSize; i++)
        {
            uint32_t l = (x * TileParams::GridSize + i) * step;
            uint32_t m = (y * TileParams::GridSize + k) * step;

            if (l >= m_size) continue;
            if (m >= m_size) continue;

            uint32_t data_idx = m * m_size + l;

            uint16_t* data = reinterpret_cast<uint16_t*>(m_heightmap->data);
            float val = data[data_idx] / 65535.0f * m_height;

            if (val < range.first) range.first = val;
            if (val > range.second) range.second = val;
        }
}

void TerrainData::generateLevelTiles(uint32_t level)
{
    uint32_t tnum = 1 << level;

    for (uint32_t y = 0; y < tnum; y++)
        for (uint32_t x = 0; x < tnum; x++)
        {
            generateTile(level, x, y);
        }
}

void TerrainData::generateTiles()
{
    for (uint32_t level = m_levels; ; level--)
    {
        generateLevelTiles(level);
        if (level == 0) break;
    }
}

const HeightRange& TerrainData::getTileRange(const TileKey& tilekey) const
{
    return m_ranges.at(tilekey);
}