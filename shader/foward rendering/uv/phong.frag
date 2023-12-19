#version 460

#define USING_VERTEX_TEXTURE_UV
#define SKYBOX_REFLECTION
// #define CUBEMAP_SKYBOX
vec3 color;

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bMaterial;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/BlinnPhongMaterial.glsl
#include functions/Reflections.glsl
#include functions/NormalMap.glsl

in vec3 viewPos;
in vec3 viewVector;

#ifndef PI
#define PI 3.1415926538
#endif

void main()
{
    vec4 CE = texture(bColor, uv);
    vec4 NRM = texture(bMaterial, uv);

    mSpecular = 0.9;
    mMetallic = 1.0 - NRM.a;
    mRoughness = NRM.b;
    mEmmisive = 1.0 - CE.a;
    color = CE.rgb;
    normalComposed = perturbNormal(normal, viewVector, NRM.xy, uv);


    // test 
        // mMetallic = 1.f;
        // mRoughness = 0.f;
        // color = vec3(0.85);
        // color = vec3(225.0, 215.0, 0.0)/255.0;
    //

    colorVCorrection = 1.0-pow(rgb2v(color), 5.0);

    viewDir = normalize(_cameraPosition - position);

    vec3 rColor = getSkyboxReflection(viewDir, normalComposed);

    Material material = getMultiLightStandard();
    vec3 materialColor = ambientLight + material.diffuse + material.specular + material.fresnel;

    #ifdef SKYBOX_REFLECTION
        // fragColor.rgb = mix(color, rColor, (1.0-mRoughness)*0.25)*materialColor;        
        // fragColor.rgb *= mix(vec3(1.0), rColor, max(mMetallic*0.9, 0.0));

        // fragColor.rgb = mix(color, rColor, (1.0-mRoughness)*0.25)*mix(materialColor, vec3(1.f) + material.specular, max(mMetallic*0.9, 0.0));

        // fragColor.rgb = mix(color, rColor, (1.0-mRoughness)*0.25)*materialColor;   

        materialColor = mix(materialColor, vec3(1.0), mMetallic*0.25);
        fragColor.rgb = mix(color*materialColor, (1.0-mRoughness)*rColor, max(mMetallic*0.75, 0.0));

    #else
         fragColor.rgb = color*materialColor;
    #endif
    
    fragColor.rgb = mix(fragColor.rgb, color / pow(ambientLight+0.5, vec3(0.3)), mEmmisive);

    fragEmmisive = (mEmmisive*25.0) * 2.0 * fragColor.rgb *(rgb2v(fragColor.rgb) - ambientLight*0.5);
    fragEmmisive += 0.5 * fragColor.rgb * (rgb2v(material.specular) - ambientLight);

    fragNormal = normalize((vec4(normalComposed, 0.0) * inverse(_cameraViewMatrix)).rgb)*0.5 + 0.5;
}