#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include globals/DiffuseSpecularRim.glsl

in vec3 viewPos;

void main()
{
    #include code/DiffuseSpecularRim.glsl
    
    fragColor = vec4(DiffuseSpecularRimResult, 1.0);

    #include code/SetFragment3DOutputs.glsl
};

