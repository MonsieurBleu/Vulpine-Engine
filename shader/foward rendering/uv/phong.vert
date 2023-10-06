#version 460

#define USING_VERTEX_TEXTURE_UV

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Vertex3DInputs.glsl
#include globals/Vertex3DOutputs.glsl

out vec3 viewPos;
out vec3 viewVector;

void main()
{
    viewPos = (_cameraViewMatrix * 0.0001 * _modelMatrix * vec4(_positionInModel, 1.0)).rgb;

    #include code/SetVertex3DOutputs.glsl
    
    viewVector = _cameraPosition - position;

    gl_Position = _cameraMatrix * vec4(position, 1.0);
};