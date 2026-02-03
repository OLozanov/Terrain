#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    vec3 pos;
} view;

layout(push_constant) uniform constants
{
    float animTime;
} params;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragAlpha;

void main() 
{
    vec4 world_pos = vec4(inPosition, 1.0);
    gl_Position = view.proj * world_pos;

    vec2 tcoord = vec4(inPosition, 1.0).xz;
    vec2 tex_offset = vec2(1, 1) * params.animTime;
   
	fragTexCoord = tcoord * 0.5 + tex_offset;
    fragAlpha = length(tcoord) - 3.0;
}