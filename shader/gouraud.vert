#version 460

#include include/uniform/Base3D.glsl
#include include/uniform/Model3D.glsl

#include include/globals/Vertex3DInputs.glsl
#include include/globals/Vertex3DOutputs.glsl

#include include/globals/DiffuseSpecularRim.glsl

out vec3 albedo;

void main()
{
    #include include/code/SetVertex3DOutputs.glsl

    #include include/code/DiffuseSpecularRim.glsl
    color = DiffuseSpecularRimResult;

    albedo = diffuseResult;
    
    gl_Position = _cameraMatrix * vec4(position, 1.0);
};