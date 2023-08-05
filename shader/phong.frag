#version 460

#include include/uniform/Base3D.glsl
#include include/uniform/Model3D.glsl

#include include/globals/Fragment3DInputs.glsl
#include include/globals/Fragment3DOutputs.glsl

#include include/globals/DiffuseSpecularRim.glsl

in vec3 viewPos;
in vec3 viewNormal;

void main()
{
    #include include/code/DiffuseSpecularRim.glsl
    
    fragColor = vec4(DiffuseSpecularRimResult, 1.0);

    // mat3 normalMatrix = transpose(inverse(mat3(_cameraViewMatrix * _modelMatrix * inverse(_modelRotation))));
    // fragNormal = normalize(normalMatrix * normal);
    fragNormal = viewNormal;
    fragAlbedo = diffuseResult;
    fragPosition = vec4(viewPos, 1.0);
};

