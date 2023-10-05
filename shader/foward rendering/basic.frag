#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

in float u;

#include functions/HSV.glsl

void main()
{
    // fragColor = vec4(color, 1.0);
    fragColor = vec4(1.0);
    #include code/SetFragment3DOutputs.glsl


    float t = 0.5 + 0.5*cos(_iTime);
    // fragColor.r = u;
    // if(u <= t)
    //     discard;
    
    // fragColor.r = 0.5 + 0.5*cos(_iTime + u*5.0);
    // fragColor.rgb = hsv2rgb(fragColor.rgb);
};