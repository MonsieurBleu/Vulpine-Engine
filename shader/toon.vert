#version 460

#include include/uniform/Base3D.glsl
#include include/uniform/Model3D.glsl

#include include/globals/Vertex3DInputs.glsl
#include include/globals/Vertex3DOutputs.glsl

void main()
{
    #include include/code/SetVertex3DOutputs.glsl
    
    gl_Position = _cameraMatrix * vec4(position, 1.0);
};