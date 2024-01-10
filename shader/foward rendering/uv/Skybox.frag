#version 460

#define USING_VERTEX_TEXTURE_UV

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bTexture;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl
#include functions/Skybox.glsl

in vec3 viewPos;

float gamma = 1.0;
float exposure = 1.0;

void main()
{
    color = getSkyColor(uv);

    // color = pow(vec3(1.0) - exp(-color*exposure), vec3(1.0/gamma));

    fragColor.rgb = color;
    // fragEmmisive = getStandardEmmisive(fragColor.rgb, ambientLight);

    fragEmmisive = 0.65*fragColor.rgb*(rgb2v(fragColor.rgb) - ambientLight);

    // fragColor.rgb = vec3(uv, 1.0);
    // fragColor.rgb = vec3(uv.x-mod(uv.x, 0.1), 0.1, 0.0);
    // fragColor.rgb = vec3(0.1, uv.y-mod(uv.y, 0.1), 0.0);

    fragNormal = vec3(1);
}
