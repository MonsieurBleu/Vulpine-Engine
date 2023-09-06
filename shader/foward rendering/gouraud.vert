#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

#include globals/Vertex3DInputs.glsl
#include globals/Vertex3DOutputs.glsl

#include functions/standardMaterial.glsl

out vec3 emmisive;

void main()
{
    #include code/SetVertex3DOutputs.glsl

    color = getMultiLightStandard();
    emmisive = getStandardEmmisive(color.rgb, ambientLight);
    
    gl_Position = _cameraMatrix * vec4(position, 1.0);
};