#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Render/Render.h"

#include "Resources/Image.h"

#include "View.h"
#include "SkyDome.h"
#include "Terrain.h"

#include "Sync.h"

#include <thread>
#include <vector>
#include <memory>

enum Key
{
    key_up = 1,
    key_down = 2,
    key_left = 4,
    key_right =	8
};

class App
{
    uint32_t m_width;
    uint32_t m_height;

    Render::SwapChain m_swapchain;

    Render::Pipeline m_skyPipeline;
    Render::Pipeline m_terrainPipeline;
    Render::Pipeline m_fogPipeline;
    Render::Pipeline m_waterPipeline;
    Render::Pipeline m_debugPipeline;
    
    Render::DescriptorSet m_skyDescriptors;
    Render::DescriptorSet m_skyReflDescriptors;
    Render::DescriptorSet m_terrainDescriptors;
    Render::DescriptorSet m_terrainReflDescriptors;
    Render::DescriptorSet m_fogDescriptors;
    std::vector<Render::DescriptorSet> m_waterDescriptors;
    Render::DescriptorSet m_debugDescriptors;

    Render::CommandList m_mainCommandList;
    Render::CommandList m_reflCommandList;

    std::unique_ptr<Image> m_grass;
    std::unique_ptr<Image> m_dirt;
    std::unique_ptr<Image> m_rock;

    std::unique_ptr<Image> m_grassNorm;
    std::unique_ptr<Image> m_dirtNorm;
    std::unique_ptr<Image> m_rockNorm;

    std::unique_ptr<Image> m_clouds;
    std::vector<std::unique_ptr<Image>> m_waves;

    Render::Sampler m_sampler;
    Render::Sampler m_clampSampler;

    // Debug BBox
    Render::VertexBuffer<glm::vec3> m_boxVBuffer;
    Render::IndexBuffer m_boxIBuffer;

    Render::Bitmap m_depth;
    Render::Bitmap m_background;       // Frame buffer copy for water refraction;
    Render::Bitmap m_reflection;
    Render::Bitmap m_reflDepth;

    Render::FrameBuffer m_reflFramebuffer;

    SkyDome m_skydome;
    Terrain m_terrain;

    View m_mainView;
    View m_reflectionView;

    Render::Camera& m_camera;

    std::thread m_reflectionThread;
    Event m_reflStartEvent;
    Event m_reflEndEvent;

    bool m_terminate = false;

    float m_ang = 0;
    glm::mat4 m_projMat;

    uint32_t m_keys = 0;
    bool m_debugDraw;
    bool m_wireframe;
    bool m_drawWater;

    float m_speed;
    float m_animTime;
    float m_waveAnimFrame;

    static constexpr float ZNear = 0.1f;
    static constexpr float ZFar = 1500.0f;

    static constexpr float AnimSpeed = 0.01f;
    static constexpr float AnimRange = 4.0f;

    static constexpr glm::vec3 SkyPos = glm::vec3(0.0f, 0.6f, 0.0f);
    static constexpr glm::vec3 BgColor = glm::vec3(0.24f, 0.36f, 0.6f);

    static constexpr glm::vec3 WaterColor = glm::vec3(0.11, 0.27, 0.21);
    static constexpr float WaterFogDensity = 0.25f;
    static constexpr float WaterLevel = 10.0f;

    static constexpr size_t WavesFrameNum = 8;

private:
    void displayReflection();
    void reflectionThread();

public:
    App(VkSurfaceKHR surface);
    ~App();

    void initBoxGeometry();

    void resize(uint32_t width, uint32_t height);

    void input(const SDL_Event& event);
    void update(float dt);
    void display();
};
