#pragma once

struct GrassVertex
{
    glm::vec2 pos;
    glm::vec2 norm;
};

struct GrassInstance
{
    glm::vec3 pos;
    glm::vec2 dir;
};