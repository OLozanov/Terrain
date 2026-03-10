#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    vec3 pos;
} view;

layout(push_constant) uniform constants
{
    vec2 offset;
} params;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inNorm;
layout(location = 2) in vec3 instPosition;
layout(location = 3) in vec2 instDir;

layout(location = 0) out vec3 frag_normal;
layout(location = 1) out float frag_ao;
layout(location = 2) out float frag_dist;

const float bend_factor = 0.35;
const float scale = 0.35;

void main()
{  
    vec3 xdir = vec3(instDir.x, 0.0, instDir.y);
    vec3 ydir = vec3(0.0, 1.0f, 0.0);
    vec3 zdir = vec3(-instDir.y, 0.0, instDir.x);

    float z = bend_factor * inPosition.y * inPosition.y;
    float ny = 2.0 * bend_factor * inPosition.y * inNorm.y;

    vec3 world_pos = (inPosition.x * xdir + inPosition.y * ydir + z * zdir) * scale
                     + instPosition 
                     + vec3(params.offset.x, 0.0, params.offset.y);

    vec3 world_norm = inNorm.x * xdir + ny * ydir + inNorm.y * zdir;

    gl_Position = view.proj * vec4(world_pos, 1.0);
    frag_normal = normalize(world_norm);
    frag_ao = min(1.0, world_pos.y * 2.0);
    frag_dist = length(world_pos - view.pos);
}