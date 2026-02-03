#include "Camera.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Render
{

Camera::Camera()
: m_pos()
, m_vrot(0)
, m_hrot(0)
{
}

Camera::Camera(float x, float y, float z)
: m_pos(x, y, z)
, m_vrot(0)
, m_hrot(0)
{
}

glm::vec3 Camera::direction() const
{
    float hCos = cos(m_hrot*glm::pi<float>()/180);
    float hSin = sin(m_hrot*glm::pi<float>()/180);

    float vCos = cos((m_vrot+90)*glm::pi<float>()/180);
    float vSin = sin((m_vrot+90)*glm::pi<float>()/180);

    return glm::vec3(hSin*vSin, vCos, -hCos*vSin);
}

void Camera::rotate(float v, float h)
{
    m_vrot += v;
    if(m_vrot > 90) m_vrot = 90;
    if(m_vrot < -90) m_vrot = -90;

    m_hrot += h;
    if(m_hrot > 360) m_hrot -= 360;
    if(m_hrot < 0) m_hrot += 360;
}

void Camera::reflect(const Camera& camera, float h)
{
    const glm::mat4 cmat = camera.m_rotMat;
    m_rotMat = glm::mat4(cmat[0], -cmat[1], cmat[2], cmat[3]);
    m_pos = camera.m_pos;
    m_pos.y -= 2.0f * (m_pos.y - h);

    m_viewMat = glm::translate(m_rotMat, { -m_pos.x, -m_pos.y, -m_pos.z });
}

void Camera::update()
{
    m_rotMat = glm::rotate(glm::mat4(1.0f), m_vrot / 180.0f * glm::pi<float>(), { 1.0f, 0.0f, 0.0f });
    m_rotMat = glm::rotate(m_rotMat, m_hrot / 180.0f * glm::pi<float>(), { 0.0f, 1.0f, 0.0f });
    m_viewMat = glm::translate(m_rotMat, {-m_pos.x, -m_pos.y, -m_pos.z});
}

} // namespace Render