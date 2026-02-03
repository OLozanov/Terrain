#version 450
#extension GL_ARB_separate_shader_objects : enable

const vec2 quad[4] = vec2[4]( vec2(-1.0f, 1.0f),
                              vec2(1.0f, 1.0f),
                              vec2(-1.0f, -1.0f),
                              vec2(1.0f, -1.0f) );

void main() 
{
    gl_Position = vec4(quad[gl_VertexIndex], 0.0, 1.0);
}