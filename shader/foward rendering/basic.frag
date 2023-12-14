#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

void main()
{
    fragColor = vec4(color, 1.0);
    fragNormal = vec3(1.f);
    // fragEmmisive.rgb = fragColor.rgb;
    fragEmmisive.rgb = vec3(0.0);
};