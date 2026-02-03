#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "View.h"

View::View(Terrain& terrain)
: m_terrainView(terrain, m_camera, m_frustum)
{
}

void View::setProjectionMat(const glm::mat4& proj, float znear, float zfar)
{ 
    m_projMat = proj;
    m_sceneConstantBuffer->znear = znear;
    m_sceneConstantBuffer->zfar = zfar;
}

void View::update()
{
    m_camera.update();

    glm::mat4 skyProj = m_projMat * glm::translate(m_camera.rotation(), SkyPos);
    glm::mat4 viewProj = m_projMat * m_camera.transform();

    m_sceneConstantBuffer->viewProj = viewProj;
    m_skyConstantBuffer->viewProj = skyProj;

    m_frustum.update(viewProj);
    m_terrainView.update();
}

void View::update(uint32_t width, uint32_t height)
{
    m_camera.update();

    glm::mat4 skyProj = m_projMat * glm::translate(m_camera.rotation(), SkyPos);
    glm::mat4 viewProj = m_projMat * m_camera.transform();

    float fovx = 1.0f / m_projMat[0][0];
    float fovy = 1.0f / m_projMat[1][1];

    glm::mat3 cameraMat = transpose(m_camera.rotation());

    glm::vec3 topleft = cameraMat * glm::vec3(-fovx, fovy, -1.0f);
    glm::vec3 xdir = cameraMat[0] * fovx * (2.0f / width);
    glm::vec3 ydir = -(cameraMat[1] * fovy * (2.0f / height));

    m_sceneConstantBuffer->viewProj = viewProj;
    m_sceneConstantBuffer->viewPos = glm::vec4(m_camera.pos(), 0.0f);
    m_sceneConstantBuffer->xdir = glm::vec4(xdir, 0.0f);
    m_sceneConstantBuffer->ydir = glm::vec4(ydir, 0.0f);
    m_sceneConstantBuffer->topleft = glm::vec4(topleft, 0.0f);

    m_skyConstantBuffer->viewProj = skyProj;

    m_frustum.update(viewProj);
}

void View::reflect(const View& view, float h)
{
    m_camera.reflect(view.camera(), h);

    glm::mat4 skyProj = m_projMat * glm::translate(m_camera.rotation(), SkyPos);
    glm::mat4 viewProj = m_projMat * m_camera.transform();

    m_sceneConstantBuffer->viewProj = viewProj;
    m_sceneConstantBuffer->viewPos = glm::vec4(m_camera.pos(), 0.0f);
    m_skyConstantBuffer->viewProj = skyProj;

    m_frustum.update(viewProj);
}