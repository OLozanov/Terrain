#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    vec3 pos;
} view;

layout(push_constant, std430) uniform constants
{
    float size;
    layout(offset = 4) float hscale;
    layout(offset = 8) uint clip;
    layout(offset = 12) float loddist;
    layout(offset = 16) mat4 modelMat;
    
} params;

layout(binding = 1) uniform sampler2D heightmap;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 fragTerCoord;
layout(location = 3) out float fragHeight;

vec2 morphVertex(vec2 gridpos, float morph)
{
	vec2 fracPart = fract(gridpos * 0.5) * 2.0;  // detect odd vertices
	return gridpos - fracPart * morph;
}

void main() 
{
    vec4 tpos = vec4(inPosition.x, 0, inPosition.y, 1.0);
    vec2 testcoord = ((params.modelMat*tpos).xz)/params.size + 0.5;

    float h = textureLod(heightmap, testcoord, 0.0).r;
    tpos.y = h * params.hscale;

    vec4 testpos = params.modelMat*tpos;
    float dist = length(testpos.xyz - view.pos);

    float morph = (dist - params.loddist) / params.loddist;
    morph = clamp(morph / 0.5 - 1.0, 0.0, 1.0);

    vec2 mpos = morphVertex(inPosition, morph);
    vec4 pos = vec4(mpos.x, 0, mpos.y, 1.0);

    vec2 ter_coord = ((params.modelMat*pos).xz)/params.size + 0.5;

    h = textureLod(heightmap, ter_coord, 0.0).r;
    pos.y = h * params.hscale;

    vec4 world_pos = params.modelMat * pos;
    gl_Position = view.proj * world_pos;
    
    fragPos = world_pos.xyz;
	fragTexCoord = (params.modelMat * pos).xz * 0.5;
    fragTerCoord = ter_coord;
    fragHeight = pos.y;

    gl_ClipDistance[0] = params.clip == 1 ? pos.y - 9.2 : 1.0;
}