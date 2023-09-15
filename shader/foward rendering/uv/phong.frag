#version 460

#define USING_VERTEX_TEXTURE_UV

vec3 color;

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bTexture;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl

in vec3 viewPos;

void main()
{
    color = texture(bTexture, vec2(uv.x, 1.0 - uv.y)).rgb;
    // color = texture(bTexture, uv).rgb;
    // color = vec3(uv, 0.0);

    #include code/SetFragment3DOutputs.glsl
    fragColor.rgb = getMultiLightStandard();
    fragEmmisive = color*getStandardEmmisive(fragColor.rgb/color, ambientLight);
}
