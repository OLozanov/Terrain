#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "App.h"

#include "Render/Vertex.h"

#include "shaders/sky.vert.h"
#include "shaders/sky.frag.h"

#include "shaders/terrain.vert.h"
#include "shaders/terrain.frag.h"

#include "shaders/fog.vert.h"
#include "shaders/fog.frag.h"

#include "shaders/water.vert.h"
#include "shaders/water.frag.h"

#include "shaders/debug.vert.h"
#include "shaders/debug.frag.h"

#include <string>
#include <cmath>

const Render::InputLayout SimpleLayout = { .bindings = { {0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX} },
                                           .attributes = { {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0} }
};

const Render::InputLayout VertexLayout = { .bindings = { {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX} },
                                           .attributes = { {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
                                                           {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)},
                                                           {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
                                                           {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)},
                                                           {4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, binormal)} }
                                         };

const Render::InputLayout TerrainLayout = { .bindings = { {0, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX} },
                                            .attributes = { {0, 0, VK_FORMAT_R32G32_SFLOAT, 0} }
                                          };

const Render::BindingLayout SimpleBindings = { .bindings = { {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} },
                                               .pushranges = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 16} }
                                             };

const Render::BindingLayout SkyBindings = { .bindings = { {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} },
                                            .pushranges = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) } }
                                          };

const Render::BindingLayout TerrainBindings = { .bindings = { {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                              {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                              {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                              {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                              {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} },
                                               .pushranges = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * (16 + 4)} }
                                              };

const Render::BindingLayout FogBindings = { .bindings = { {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                          {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}},
                                            .pushranges = { {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 6} }
                                          };

const Render::BindingLayout WaterBindings = { .bindings = { {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} },
                                              .pushranges = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) },
                                                              {VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float), sizeof(uint32_t) * 3 },}
                                            };

const Render::BindingLayout DebugBindings = { .bindings = { {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr} },
                                              .pushranges = { {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 16} }
                                            };

App::App(VkSurfaceKHR surface)
: m_swapchain(surface)
, m_skyPipeline(g_sky_vert, g_sky_vert_size, g_sky_frag, g_sky_frag_size, SimpleLayout, SkyBindings, 
                { .depthTest = VK_FALSE,
                  .depthWrite = VK_FALSE,
                  .blend = VK_TRUE,
                  .dynamicCullMode = true })
, m_terrainPipeline(g_terrain_vert, g_terrain_vert_size, g_terrain_frag, g_terrain_frag_size, TerrainLayout, TerrainBindings, 
                    { .dynamicCullMode = true })
, m_fogPipeline(g_fog_vert, g_fog_vert_size, g_fog_frag, g_fog_frag_size, {}, FogBindings,
                { .primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                  .depthTest = VK_FALSE,
                  .depthWrite = VK_FALSE,
                  .blend = VK_TRUE })
, m_waterPipeline(g_water_vert, g_water_vert_size, g_water_frag, g_water_frag_size, {}, WaterBindings, 
                  { .cullMode = VK_CULL_MODE_NONE,
                    .primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP })
, m_debugPipeline(g_debug_vert, g_debug_vert_size, g_debug_frag, g_debug_frag_size, SimpleLayout, DebugBindings, 
                  { .primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                    .depthTest = VK_TRUE,
                    .depthWrite = VK_FALSE })
, m_skyDescriptors(m_skyPipeline.descriptorLayout())
, m_skyReflDescriptors(m_skyPipeline.descriptorLayout())
, m_terrainDescriptors(m_terrainPipeline.descriptorLayout())
, m_terrainReflDescriptors(m_terrainPipeline.descriptorLayout())
, m_fogDescriptors(m_fogPipeline.descriptorLayout())
, m_debugDescriptors(m_debugPipeline.descriptorLayout())
, m_grass(LoadImage("textures/grass.png"))
, m_dirt(LoadImage("textures/dirt.png"))
, m_rock(LoadImage("textures/rock.png"))
, m_grassNorm(LoadImage("textures/grass1_n.png"))
, m_dirtNorm(LoadImage("textures/dirt_n.png"))
, m_rockNorm(LoadImage("textures/rock_n.png"))
, m_clouds(LoadImage("textures/clouds.png"))
, m_clampSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
, m_depth(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
, m_background(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
, m_reflection(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
, m_reflDepth(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
, m_skydome(24, 16, 10.0f, 25.0f)
, m_terrain()
, m_mainView(m_terrain)
, m_reflectionView(m_terrain)
, m_camera(m_mainView.camera())
, m_reflectionThread(&App::reflectionThread, this)
, m_debugDraw(false)
, m_wireframe(false)
, m_drawWater(true)
, m_speed(15.0f)
, m_animTime(0.0f)
, m_waveAnimFrame(0.0f)
{
    m_waves.resize(WavesFrameNum);

    for (size_t i = 0; i < WavesFrameNum; i++)
    {
        std::string path = std::string("textures/waves") + std::to_string(i) + ".png";
        m_waves[i].reset(LoadImage(path.c_str()));
    }

    initBoxGeometry();

    const VkExtent2D& frameExtent = m_swapchain.frameExtent();

    m_depth.reset(frameExtent.width, frameExtent.height);
    m_background.reset(frameExtent.width, frameExtent.height);
    m_reflection.reset(frameExtent.width, frameExtent.height);
    m_reflDepth.reset(frameExtent.width, frameExtent.height);

    m_reflection.setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_reflFramebuffer.resize(frameExtent);
    m_reflFramebuffer.addColorAttachment(m_reflection);
    m_reflFramebuffer.addDepthAttachment(m_reflDepth);
    m_reflFramebuffer.setClearColor(BgColor.r, BgColor.g, BgColor.b);

    m_skyDescriptors.bind(0, m_mainView.skyConstantBuffer(), sizeof(ViewConstantBuffer));
    m_skyDescriptors.bind(1, *m_clouds, m_sampler);

    m_skyReflDescriptors.bind(0, m_reflectionView.skyConstantBuffer(), sizeof(ViewConstantBuffer));
    m_skyReflDescriptors.bind(1, *m_clouds, m_sampler);

    m_terrainDescriptors.bind(0, m_mainView.sceneConstantBuffer(), sizeof(ViewConstantBuffer));
    m_terrainDescriptors.bind(1, m_terrain.heightmap(), m_clampSampler);
    m_terrainDescriptors.bind(2, m_terrain.normals(), m_clampSampler);
    m_terrainDescriptors.bind(3, 0, *m_grass, m_sampler);
    m_terrainDescriptors.bind(3, 1, *m_dirt, m_sampler);
    m_terrainDescriptors.bind(3, 2, *m_rock, m_sampler);
    m_terrainDescriptors.bind(4, 0, *m_grassNorm, m_sampler);
    m_terrainDescriptors.bind(4, 1, *m_dirtNorm, m_sampler);
    m_terrainDescriptors.bind(4, 2, *m_rockNorm, m_sampler);

    m_terrainReflDescriptors.bind(0, m_reflectionView.sceneConstantBuffer(), sizeof(ViewConstantBuffer));
    m_terrainReflDescriptors.bind(1, m_terrain.heightmap(), m_clampSampler);
    m_terrainReflDescriptors.bind(2, m_terrain.normals(), m_clampSampler);
    m_terrainReflDescriptors.bind(3, 0, *m_grass, m_sampler);
    m_terrainReflDescriptors.bind(3, 1, *m_dirt, m_sampler);
    m_terrainReflDescriptors.bind(3, 2, *m_rock, m_sampler);
    m_terrainReflDescriptors.bind(4, 0, *m_grassNorm, m_sampler);
    m_terrainReflDescriptors.bind(4, 1, *m_dirtNorm, m_sampler);
    m_terrainReflDescriptors.bind(4, 2, *m_rockNorm, m_sampler);

    m_fogDescriptors.bind(0, m_mainView.sceneConstantBuffer(), sizeof(ViewConstantBuffer));
    m_fogDescriptors.bind(1, m_depth, m_clampSampler);

    m_waterDescriptors.reserve(WavesFrameNum);

    for (size_t i = 0; i < WavesFrameNum; i++)
    {
        m_waterDescriptors.emplace_back(m_waterPipeline.descriptorLayout());

        m_waterDescriptors[i].bind(0, m_mainView.sceneConstantBuffer(), sizeof(ViewConstantBuffer));
        m_waterDescriptors[i].bind(1, m_depth, m_clampSampler);
        m_waterDescriptors[i].bind(2, m_background, m_clampSampler);
        m_waterDescriptors[i].bind(3, m_reflection, m_clampSampler);
        m_waterDescriptors[i].bind(4, *m_waves[i], m_sampler);
    }

    m_debugDescriptors.bind(0, m_mainView.sceneConstantBuffer(), sizeof(ViewConstantBuffer));

    resize(frameExtent.width, frameExtent.height);
    m_mainView.setProjectionMat(m_projMat, ZNear, ZFar);
    m_reflectionView.setProjectionMat(m_projMat, ZNear, ZFar);

    m_camera.setPos(glm::vec3(0.0f, 50.0f, 0.0f));

    m_reflectionThread.detach();
}

App::~App()
{
    m_terminate = true;
    m_reflStartEvent.signal();
}

void App::initBoxGeometry()
{
    std::vector<glm::vec3> vertices = { {-1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},  {1.0f, 1.0f, 1.0f},  {1.0f, 1.0f, -1.0f},
                                        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f} };
    
    std::vector<uint16_t> indices = { 0, 1, 1, 2, 2, 3, 3, 0, 
                                      4, 5, 5, 6, 6, 7, 7, 4,
                                      0, 4, 1, 5, 2, 6, 3, 7};

    m_boxVBuffer.setData(vertices.data(), vertices.size());
    m_boxIBuffer.setData(indices.data(), indices.size());
}

void App::resize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    m_projMat = glm::perspective(glm::radians(70.0f), m_width / float(m_height), ZNear, ZFar);
}

void App::input(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_EVENT_MOUSE_MOTION:
            m_camera.rotate(event.motion.yrel * 0.5, event.motion.xrel * 0.5);
        break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_W) m_keys |= key_up;
            if (event.key.key == SDLK_S) m_keys |= key_down;
            if (event.key.key == SDLK_A) m_keys |= key_left;
            if (event.key.key == SDLK_D) m_keys |= key_right;
        break;

        case SDL_EVENT_KEY_UP:
            if (event.key.key == SDLK_W) m_keys &= ~key_up;
            if (event.key.key == SDLK_S) m_keys &= ~key_down;
            if (event.key.key == SDLK_A) m_keys &= ~key_left;
            if (event.key.key == SDLK_D) m_keys &= ~key_right;
            if (event.key.key == SDLK_1) m_debugDraw = !m_debugDraw;
            if (event.key.key == SDLK_2) m_wireframe = !m_wireframe;
            if (event.key.key == SDLK_3) m_drawWater = !m_drawWater;
        break;
    }
}

void App::update(float dt)
{
    glm::vec3 dir = m_camera.direction();

    if (m_keys & key_up) m_camera.move(dir * m_speed * dt);
    if (m_keys & key_down) m_camera.move(-dir * m_speed * dt);

    m_animTime = std::fmod(m_animTime + dt * AnimSpeed, AnimRange);
    m_waveAnimFrame = std::fmod(m_waveAnimFrame + dt * 8.0f, float(WavesFrameNum));
}

void App::displayReflection()
{
    m_reflectionView.updateVisibility();

    Render::VulkanInstance& vkInstance = Render::VulkanInstance::GetInstance();
    m_reflCommandList.begin();

    // Explicit layout transition here due to thread concurency
    m_reflCommandList.barrier(m_reflection, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_reflCommandList.barrier(m_reflDepth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    m_reflCommandList.bindFrameBuffer(m_reflFramebuffer);

    m_reflCommandList.setViewport(m_width, m_height);
    m_reflCommandList.setPolygonMode(VK_POLYGON_MODE_FILL);
    m_reflCommandList.setCullMode(VK_CULL_MODE_FRONT_BIT);

    // Sky
    m_reflCommandList.bindPipeline(m_skyPipeline);
    m_reflCommandList.bindDescriptorSet(m_skyReflDescriptors);

    m_reflCommandList.setConstant(0, m_animTime);
    m_skydome.display(m_reflCommandList);

    // Terrain
    m_reflCommandList.bindPipeline(m_terrainPipeline);
    m_reflCommandList.bindDescriptorSet(m_terrainReflDescriptors);
    m_reflCommandList.setConstant(8, VkBool32(VK_TRUE));

    m_reflectionView.displayTerrain(m_reflCommandList);

    m_reflCommandList.finishRender();
    m_reflCommandList.finish();
    vkInstance.submit(m_reflCommandList);
}

void App::reflectionThread()
{
    while (true)
    {
        m_reflStartEvent.wait();

        if (m_terminate) break;

        displayReflection();
        m_reflEndEvent.signal();
    }
}

void App::display()
{
    Render::VulkanInstance& vkInstance = Render::VulkanInstance::GetInstance();

    uint32_t bufferIndex = m_swapchain.acquireBuffer();

    bool drawWater = !m_wireframe && m_drawWater;

    m_mainView.update(m_width, m_height);
    m_reflectionView.reflect(m_mainView, WaterLevel);

    m_mainView.updateVisibility();

    if (drawWater) m_reflStartEvent.signal(); 

    m_mainCommandList.begin();

    if (m_wireframe) 
        m_mainCommandList.clearColor(0.0f, 0.0f, 0.0f);
    else 
        m_mainCommandList.clearColor(BgColor.r, BgColor.g, BgColor.b);

    m_mainCommandList.barrier(m_swapchain.image(bufferIndex), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_mainCommandList.barrier(m_depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    m_mainCommandList.bindFrameBuffer(m_swapchain.frameExtent(), m_swapchain.colorBuffer(bufferIndex), m_depth);

    m_mainCommandList.setViewport(m_width, m_height);
    m_mainCommandList.setPolygonMode(m_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL);
    m_mainCommandList.setCullMode(VK_CULL_MODE_BACK_BIT);

    // Sky
    if (!m_wireframe)
    {
        m_mainCommandList.bindPipeline(m_skyPipeline);
        m_mainCommandList.bindDescriptorSet(m_skyDescriptors);

        m_mainCommandList.setConstant(0, m_animTime);
        m_skydome.display(m_mainCommandList);
    }

    // Terrain
    m_mainCommandList.bindPipeline(m_terrainPipeline);
    m_mainCommandList.bindDescriptorSet(m_terrainDescriptors);
    m_mainCommandList.setConstant(8, VkBool32(VK_FALSE));

    m_mainView.displayTerrain(m_mainCommandList);

    // Water
    if (drawWater)
    {
        float fogParams[5] = { WaterColor.x, WaterColor.y, WaterColor.z, WaterFogDensity, WaterLevel };

        m_mainCommandList.finishRender();

        m_mainCommandList.barrier(m_depth, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_mainCommandList.bindFrameBuffer(m_swapchain.frameExtent(), m_swapchain.colorBuffer(bufferIndex));

        m_mainCommandList.bindPipeline(m_fogPipeline);
        m_mainCommandList.bindDescriptorSet(m_fogDescriptors);

        m_mainCommandList.setConstant(0, fogParams, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_mainCommandList.draw(4);

        m_mainCommandList.finishRender();

        m_mainCommandList.barrier(m_swapchain.image(bufferIndex), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        m_mainCommandList.barrier(m_background, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_mainCommandList.copyImage(m_swapchain.image(bufferIndex), m_background, m_width, m_height);
        m_mainCommandList.barrier(m_swapchain.image(bufferIndex), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        m_mainCommandList.barrier(m_background, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_mainCommandList.barrier(m_reflection, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_mainCommandList.bindFrameBuffer(m_swapchain.frameExtent(), m_swapchain.colorBuffer(bufferIndex));
        m_mainCommandList.bindPipeline(m_waterPipeline);
        m_mainCommandList.bindDescriptorSet(m_waterDescriptors[m_waveAnimFrame]);

        uint32_t reflectionParams[3] = { m_camera.pos().y > WaterLevel ? 1 : 0, m_width, m_height };
        m_mainCommandList.setConstant(4, reflectionParams, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_mainCommandList.draw(4);
    }

    if (m_debugDraw)
    {
        m_mainCommandList.bindPipeline(m_debugPipeline);
        m_mainCommandList.bindDescriptorSet(m_debugDescriptors);

        m_mainCommandList.setPolygonMode(VK_POLYGON_MODE_LINE);
        
        m_mainCommandList.bindIndexBuffer(m_boxIBuffer);
        m_mainCommandList.bindVertexBuffer(m_boxVBuffer);
        m_mainView.displayBBoxes(m_mainCommandList);
    }

    m_mainCommandList.finishRender();
    m_mainCommandList.barrier(m_swapchain.image(bufferIndex), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    m_mainCommandList.finish();
    
    if (drawWater) m_reflEndEvent.wait();
    vkInstance.submit(m_mainCommandList);
    m_swapchain.present();
}
