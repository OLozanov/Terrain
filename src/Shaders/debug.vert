#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    vec3 pos;
} view;

layout( push_constant ) uniform constants
{
    mat4 mat;
} model;

layout(location = 0) in vec3 inPosition;

void main() 
{
    vec4 world_pos = model.mat * vec4(inPosition, 1.0);
    gl_Position = view.proj * world_pos;
}