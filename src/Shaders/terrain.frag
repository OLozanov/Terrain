#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 fragTerCoord;
layout(location = 3) in float fragHeight;

layout(binding = 2) uniform sampler2D normals;          // geometry normals
layout(binding = 3) uniform sampler2D diffuse_map[3];
layout(binding = 4) uniform sampler2D normal_map[3];

layout(location = 0) out vec4 outColor;

const vec3 lightDir = normalize(vec3(0.8, 1, 0.8));

const float DirtLevel = 10.0;
const float RockLevel = 30.0;
const float Transition = 5.0;

void main() 
{
    uint tid1 = 0;
    uint tid2 = 0;
    float blend = 1.0;

    if (fragHeight < DirtLevel)
    {
        tid1 = 1;
        tid2 = 1;
    }
    else if (fragHeight < DirtLevel + Transition)
    {
        tid1 = 1;
        tid2 = 0;

        blend = (fragHeight - DirtLevel) / Transition;
    }

    if (fragHeight > RockLevel + Transition)
    {
        tid1 = 2;
        tid2 = 2;
    }
    else if (fragHeight > RockLevel)
    {
        tid1 = 0;
        tid2 = 2;

        blend = (fragHeight - RockLevel) / Transition;
    }

    vec3 diffuse1 = texture(diffuse_map[tid1], fragTexCoord).xyz;
    vec3 normal1 = texture(normal_map[tid1], fragTexCoord).xyz * 2.0 - 1.0;

    vec3 diffuse2 = texture(diffuse_map[tid2], fragTexCoord).xyz;
    vec3 normal2 = texture(normal_map[tid2], fragTexCoord).xyz * 2.0 - 1.0;

    vec3 diffuse = mix(diffuse1, diffuse2, blend);
    vec3 normal = mix(normal1, normal2, blend);

    // Tangent space
    vec3 tspacez = textureLod(normals, fragTerCoord, 0.0).xzy * 2.0 - 1.0;
    vec3 tspacex = normalize(vec3(1, 0, 0) - tspacez * tspacez.x);
    vec3 tspacey = normalize(vec3(0, 0, 1) - tspacez * tspacez.z);

    //vec3 tspacex = normalize(xdir - tspacez * dot(tspacez, xdir));
    //vec3 tspacey = normalize(ydir - tspacez * dot(tspacez, ydir));

    vec3 world_normal = normal.x * tspacex + normal.y * tspacey + normal.z * tspacez;

    float ltCos = 0.5 + max(0.0, dot(lightDir, world_normal)) * 0.5;
    outColor = vec4(diffuse, 1.0) * ltCos * ltCos;
}