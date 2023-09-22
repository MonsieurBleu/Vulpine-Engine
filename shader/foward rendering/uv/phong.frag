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
    color = texture(bTexture, vec2(uv.x, 1.0 - uv.y)).rgb;

    vec3 nNormal = normalize(normal);
    vec3 viewDir = normalize(_cameraPosition - position);
    vec3 reflectDir = reflect(viewDir, nNormal); 

    float r = pow(max(dot(reflectDir, viewDir), 0.0), 16.0);

    float angle1 = acos(reflectDir.x)*sign(asin(reflectDir.y));
    float angle2 = -acos(reflectDir.z)*sign(asin(reflectDir.y));

    angle1 = (angle1)/(PI);
    angle2 = (angle2)/(PI);
    vec2 uv = vec2(angle1, angle2);

    vec3 rColor = texture(bSkyTexture, uv).rgb;

    #include code/SetFragment3DOutputs.glsl
    fragColor.rgb = 0.0*getMultiLightStandard() + rColor;
    fragEmmisive = color*getStandardEmmisive(fragColor.rgb/color, ambientLight);
}
