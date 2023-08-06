#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Vertex3DInputs.glsl
#include globals/Vertex3DOutputs.glsl

#include globals/DiffuseSpecularRim.glsl

void main()
{
    #include code/SetVertex3DOutputs.glsl

    #include code/DiffuseSpecularRim.glsl
    color = DiffuseSpecularRimResult;
    
    gl_Position = _cameraMatrix * vec4(position, 1.0);
};