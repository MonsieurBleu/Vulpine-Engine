#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Vertex3DInputs.glsl
#include globals/Vertex3DOutputs.glsl

out vec3 viewPos;

void main()
{
    viewPos = (_cameraViewMatrix * 0.0001 * _modelMatrix * vec4(_positionInModel, 1.0)).rgb;

    #include code/SetVertex3DOutputs.glsl

    gl_Position = _cameraMatrix * vec4(position, 1.0);
};