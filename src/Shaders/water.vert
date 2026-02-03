#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    vec3 pos;
} view;

const vec2 quad[4] = vec2[4]( vec2(-1.0f, 1.0f),
                              vec2(1.0f, 1.0f),
                              vec2(-1.0f, -1.0f),
                              vec2(1.0f, -1.0f) );

layout(push_constant) uniform constants
{
    float level;
} params;

layout(location = 0) out vec2 tcoord;
layout(location = 1) out vec3 view_vec;

const float size = 3000;
const float tex_scale = 0.125; 

void main() 
{
    vec4 world_pos = vec4(quad[gl_VertexIndex].x * size + view.pos.x, 10.0, quad[gl_VertexIndex].y * size + view.pos.z, 1.0);
    gl_Position = view.proj * world_pos;

    tcoord = world_pos.xz * tex_scale;
    view_vec = world_pos.xyz - view.pos;
}