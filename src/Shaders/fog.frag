#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    vec3 pos;
    mat3 viewmat;
    float znear;
    float zfar;
} view;

layout(push_constant) uniform constants
{
    vec3 color;
    float density;
    float level;
} params;

layout(binding = 1) uniform sampler2D depth;

layout(location = 0) out vec4 outColor;

float getLinearDepth(float depth)
{
    const float near = view.znear;
    const float far = view.zfar;

    return near * far / (far + depth * (near - far));
}

void main() 
{
    const float near = view.znear;
    const float far = view.zfar;

    const float h = params.level;

    float d = texelFetch(depth, ivec2(gl_FragCoord.xy), 0).r;

    vec3 view_vec = view.viewmat * vec3(gl_FragCoord.xy + 0.5, 1.0);
    float rlen = 1.0/length(view_vec);
    vec3 dir = view_vec * rlen;

    if (dir.y > -0.0001 && view.pos.y > h) discard;
    float dist = (view.pos.y - h) / -dir.y;
    dist *= rlen;

    dist = view.pos.y > h ? max(near, dist) : dir.y < 0 ? far : dist;

    float depth = getLinearDepth(d);

    float tmin = view.pos.y > h ? max(near, dist) : min(near, depth);
    float tmax = view.pos.y > h ? max(dist, depth) : min(depth, dist);

    float fdist = max(0.0, tmax - tmin);
    float fog_factor = exp(-fdist * params.density);

    outColor = vec4(params.color, 1.0 - fog_factor);
}