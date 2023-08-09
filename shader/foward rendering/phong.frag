#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

// inclue globals/DiffuseSpecularRim.glsl

#include functions/standartMaterial.glsl

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}


in vec3 viewPos;

void main()
{
    #include code/SetFragment3DOutputs.glsl
    // include code/DiffuseSpecularRim.glsl
    
    // fragColor = vec4(DiffuseSpecularRimResult, 1.0);

    // ambientLight = vec3(cos(_iTime*0.25)*0.5 + 0.5);

    fragColor.rgb = getMultiLightDFS();
    fragEmmisive = 
        fragColor.rgb * 
        max(
            rgb2hsv(fragColor.rgb).b - min(max(ambientLight*1.5, 0.1), 0.8), 
            0.0);
    // fragEmmisive = fragColor.rgb;
}
