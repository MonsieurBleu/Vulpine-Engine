#version 460

#define USING_VERTEX_TEXTURE_UV
#define SKYBOX_REFLECTION
// #define CUBEMAP_SKYBOX

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bMaterial;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl
#include functions/Reflections.glsl
#include functions/NormalMap.glsl

in vec3 viewPos;
in vec3 viewVector;

void main()
{
    vec4 CE = texture(bColor, uv);
    vec4 NRM = texture(bMaterial, uv);

    if(NRM.x <= 0.01 && NRM.y <= 0.01) discard;

    mEmmisive = 1.0 - CE.a;
    mMetallic = 1.0 - NRM.a;
    mRoughness = NRM.b;
    mRoughness2 = mRoughness*mRoughness;
    color = CE.rgb;
    normalComposed = perturbNormal(normal, viewVector, NRM.xy, uv);
    viewDir = normalize(_cameraPosition - position);
    
    Material material = getMultiLightPBR();
    vec3 rColor = getSkyboxReflection(viewDir, normalComposed);
    const float reflectFactor = getReflectionFactor(material.fresnel, mMetallic, mRoughness);
    fragColor.rgb = color*ambientLight + material.result + rColor*reflectFactor;

    fragColor.rgb = mix(fragColor.rgb, color, mEmmisive);
    fragEmmisive = getStandardEmmisive(fragColor.rgb);
    
    // fragNormal = normalize((vec4(normalComposed, 0.0) * _cameraInverseViewMatrix).rgb)*0.5 + 0.5;
    fragNormal = normalize((vec4(normalComposed, 0.0) * inverse(_cameraViewMatrix)).rgb)*0.5 + 0.5;
}



