#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

// inclue globals/DiffuseSpecularRim.glsl

#include functions/standartMaterial.glsl

in vec3 viewPos;

void main()
{
    #include code/SetFragment3DOutputs.glsl
    // include code/DiffuseSpecularRim.glsl
    
    // fragColor = vec4(DiffuseSpecularRimResult, 1.0);

    fragColor.rgb = getMultiLightDFS();
}
