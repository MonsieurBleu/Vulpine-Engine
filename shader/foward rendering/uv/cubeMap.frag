#version 460

#define USING_VERTEX_TEXTURE_UV

vec3 color;

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bTexture;
uniform samplerCube bCubeMap; // cubemap texture sampler

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl


in vec3 viewPos;

float gamma = 1.0;
float exposure = 1.0;

void main()
{
    color = texture(bCubeMap, normalize(position - _cameraPosition)).rgb;

    fragColor.rgb = color;
    fragEmmisive = getStandardEmmisive(fragColor.rgb, ambientLight);
}
