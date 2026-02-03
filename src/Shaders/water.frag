#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 tcoord;
layout(location = 1) in vec3 view_vec;

layout(push_constant) uniform constants
{
   layout(offset = 4) bool reflection;
   layout(offset = 8) uint width;
   layout(offset = 12) uint height;
} params;

layout(binding = 1) uniform sampler2D depth;
layout(binding = 2) uniform sampler2D background;
layout(binding = 3) uniform sampler2D reflection;
layout(binding = 4) uniform sampler2D normal;

layout(location = 0) out vec4 outColor;

float FresnelSchlick(float cosv)
{
    const float R0 = 0.02; // Refraction indices: 1.333 - water, 1 - air

    return R0 + (1.0 - R0) * pow(1.0 - cosv, 5.0);
}

void main() 
{
    float d = texelFetch(depth, ivec2(gl_FragCoord.xy), 0).r;

    if (gl_FragCoord.z > d) discard;

    vec3 v = normalize(-view_vec);

    vec3 normal = texture(normal, tcoord).xyz * 2.0 - 1.0;
    vec2 dist_coord = clamp(gl_FragCoord.xy + normal.xy * 20.0, vec2(0, 0), vec2(params.width - 1, params.height - 1));

    vec3 bgcolor = texelFetch(background, ivec2(dist_coord), 0).xyz;
    vec3 rcolor = texelFetch(reflection, ivec2(dist_coord), 0).xyz;

    if (params.reflection)
    {
        vec3 norm = vec3(normal.x, normal.z * 50.0, normal.y);
        norm = normalize(norm);
        float cosv = clamp(dot(v, norm), 0.0, 1.0);
        float factor = FresnelSchlick(cosv);
        vec3 color = mix(bgcolor, rcolor, factor);

        outColor = vec4(color, 1.0);
    }
    else
        outColor = vec4(bgcolor, 1.0);
}