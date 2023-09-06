#version 460

#define TOON

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl

in vec3 viewPos;

void main()
{
    #include code/SetFragment3DOutputs.glsl
    fragColor.rgb = getMultiLightStandard();
    fragEmmisive = getStandardEmmisive(fragColor.rgb, ambientLight);
}
