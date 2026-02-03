#pragma once

#include "Render/Render.h"
#include "Terrain.h"

struct ViewConstantBuffer
{
    glm::mat4 viewProj;
    glm::vec4 viewPos;
    glm::vec4 xdir;
    glm::vec4 ydir;
    glm::vec4 topleft;
    float znear;
    float zfar;
};

class View
{
public:
    View(Terrain& terrain);

    void setProjectionMat(const glm::mat4& proj, float znear, float zfar);

    Render::Camera& camera() { return m_camera; }
    const Render::Camera& camera() const { return m_camera; }

    Render::Frustum& frustum() { return m_frustum; }
    const Render::Frustum& frustum() const { return m_frustum; }

    const auto& skyConstantBuffer() const { return m_skyConstantBuffer; }
    const auto& sceneConstantBuffer() const { return m_sceneConstantBuffer; }

    void update();
    void update(uint32_t width, uint32_t height);

    void reflect(const View& view, float h);

    void updateVisibility() { m_terrainView.update(); }

    void displayTerrain(Render::CommandList& commandList) const { m_terrainView.display(commandList); }
    void displayBBoxes(Render::CommandList& commandList) const { m_terrainView.displayBBoxes(commandList); }

private:
    glm::mat4 m_projMat;

    Render::Camera m_camera;
    Render::Frustum m_frustum;

    Render::ConstantBuffer<ViewConstantBuffer> m_skyConstantBuffer;
    Render::ConstantBuffer<ViewConstantBuffer> m_sceneConstantBuffer;

    TerrainView m_terrainView;

    static constexpr glm::vec3 SkyPos = glm::vec3(0.0f, 0.8f, 0.0f);
};