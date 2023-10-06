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

float gamma = 1.0;
float exposure = 1.0;

void main()
{
    color = texture(bTexture, vec2(uv.x, uv.y)).rgb;

    // color = pow(vec3(1.0) - exp(-color*exposure), vec3(1.0/gamma));

    fragColor.rgb = color;
    fragEmmisive = getStandardEmmisive(fragColor.rgb, ambientLight);

    // fragColor.rgb = vec3(uv, 1.0);
    // fragColor.rgb = vec3(uv.x-mod(uv.x, 0.1), 0.1, 0.0);
    // fragColor.rgb = vec3(0.1, uv.y-mod(uv.y, 0.1), 0.0);
}
