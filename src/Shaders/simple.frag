#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(binding = 1) uniform sampler2D diffuse;

layout(location = 0) out vec4 outColor;

const vec3 lightDir = normalize(vec3(1, 1, 1));

void main() 
{
    float ltCos = 0.5 + max(0.0, dot(lightDir, fragNormal)) * 0.5;
    outColor = texture(diffuse, fragTexCoord) * ltCos;
}