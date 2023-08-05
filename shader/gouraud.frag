#version 460

#include include/uniform/Base3D.glsl
#include include/uniform/Model3D.glsl

#include include/globals/Fragment3DInputs.glsl
#include include/globals/Fragment3DOutputs.glsl

in vec3 albedo;

void main()
{
    fragColor = vec4(color, 1.0);
    fragNormal = normal;
    fragAlbedo = albedo;
    fragPosition = (vec4(position, 1.0)*_cameraViewMatrix);
};