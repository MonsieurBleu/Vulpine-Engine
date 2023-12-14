#version 460

#define USING_VERTEX_TEXTURE_UV

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bMaterial;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl

in vec3 viewPos;
in vec3 viewVector;

void main()
{
}
