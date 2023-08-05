#version 460

#include include/uniform/Base3D.glsl
#include include/uniform/Model3D.glsl

#include include/globals/Vertex3DInputs.glsl
#include include/globals/Vertex3DOutputs.glsl

out vec3 viewPos;
out vec3 viewNormal;

void main()
{
    viewPos = (_cameraViewMatrix * 0.0001 * _modelMatrix * vec4(_positionInModel, 1.0)).rgb;
    mat3 normalMatrix = transpose(inverse(mat3(_cameraViewMatrix * _modelMatrix)));
    viewNormal = normalize(normalMatrix * _normal);

    #include include/code/SetVertex3DOutputs.glsl

    gl_Position = _cameraMatrix * vec4(position, 1.0);
};