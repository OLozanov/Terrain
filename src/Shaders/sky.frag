#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragAlpha;

layout(binding = 1) uniform sampler2D diffuse;

layout(location = 0) out vec4 outColor;

void main() 
{
    float alpha = 1.0 - clamp(fragAlpha, 0.0, 1.0);

    vec3 color = texture(diffuse, fragTexCoord).xyz;
    outColor = vec4(color, alpha * alpha);
}