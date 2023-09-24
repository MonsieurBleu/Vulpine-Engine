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



#define PI 3.1415926538

void main()
{
    // mSpecular = 0.5 + 0.5*cos(_iTime);
    // mRoughness = 0.5 + 0.5*cos(_iTime);
    // mMetallic = 0.5 + 0.5*cos(_iTime);

    color = texture(bTexture, vec2(uv.x, 1.0 - uv.y)).rgb;

    vec3 nNormal = normalize(normal);
    vec3 viewDir = normalize(_cameraPosition - position);
    vec3 reflectDir = reflect(viewDir, nNormal); 
    // vec3 reflectDir = refract(viewDir, nNormal, 1.0/1.52);

    float r = pow(max(dot(reflectDir, viewDir), 1.0 - mRoughness), 5.0);

    vec2 uvSky = vec2(0.0);
    uvSky.x = 0.5 + atan(reflectDir.z, -reflectDir.x) / (2.0*PI);
    uvSky.y = -reflectDir.y*0.5 + 0.5;

    vec3 rColor = (1.0 - mRoughness)*r*texture(bSkyTexture, uvSky).rgb; 

    #include code/SetFragment3DOutputs.glsl
    Material material = getMultiLightStandard();
    vec3 materialColor = ambientLight + material.diffuse + material.specular + material.fresnel;
    fragColor.rgb = color*materialColor + materialColor*rColor;
    fragColor.rgb = mix(color, rColor, max(mMetallic - 0.1, 0.0))*materialColor;

    fragEmmisive = fragColor.rgb * 2.0 * (rgb2v(fragColor.rgb) - ambientLight*0.5);

    // vec3 e = material.diffuse + material.fresnel + rColor;
    // fragEmmisive = 0.5 * pow(color, vec3(1.0)) * e * (rgb2v(e) - ambientLight*0.5);
    // fragEmmisive = 0.5 * pow(color, vec3(0.5)) * e * (rgb2v(e) - ambientLight*0.5);
    // fragEmmisive = pow(fragEmmisive, vec3(5.0));

    // vec3 e = materialColor + 0.25*rColor;
    // fragEmmisive = 0.5*pow(color, vec3(0.35))*(rgb2v(e) - ambientLight);
}
