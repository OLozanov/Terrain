#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in float frag_ao;
layout(location = 2) in float frag_dist;

//layout(binding = 1) uniform sampler2D diffuse;

layout(location = 0) out vec4 outColor;

const vec3 lightDir = normalize(vec3(0.8, 1, 0.8));
const vec3 grassColor = vec3(0.35, 0.53, 0.1);
//const vec3 grassColor = vec3(0, 1, 0);

void main() 
{
    if (frag_dist > 40.0) discard;

    float ltCos = 0.6 + max(0.0, dot(lightDir, frag_normal)) * 0.4;
    float lit = mix(0.4, ltCos * ltCos, frag_ao);
    outColor = vec4(grassColor * lit, 1.0);
}