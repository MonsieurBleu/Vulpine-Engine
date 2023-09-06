#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

vec3 lightDirection = normalize(vec3(-1.0, 1.0, 1.0));
vec3 ambientLight = vec3(0.5);

#define DIFFUSE
float diffuseIntensity = 0.40;
vec3 diffuseColor = vec3(1.0);

#define SPECULAR
float specularIntensity = 0.20;
vec3 specularColor = vec3(1.0);

#define RIM
float rimIntensity = 0.65;
vec3 rimColor = vec3(1.0);

void main()
{
    fragColor.rgb = color;

    /*
        UTILS
    */
    vec3 viewDir = normalize(_cameraPosition - position);
    vec3 reflectDir = reflect(-lightDirection, normal); 
    float nDotL = max(dot(lightDirection, normal), 0);

    /*
        DIFFUSE 
    */
    float diffuse = 0.0;
#ifdef DIFFUSE
    diffuse = nDotL;
    float dthreshold = 0.0;
    float dstepsize = 0.01;
    diffuse = smoothstep(dthreshold, dthreshold+dstepsize, diffuse);
#endif
    vec3 diffuseResult = diffuse*diffuseIntensity*diffuseColor;

    /*
        SPECULAR
    */
    float specular = 0.0;
#ifdef SPECULAR
    int specularExponent = 8;
    specular = pow(max(dot(reflectDir, viewDir), 0.0), specularExponent);
    float sthreshold = 0.25;
    float sstepsize = 0.01;
    specular = smoothstep(sthreshold, sthreshold+sstepsize, specular);
#endif
    vec3 specularResult = specular*specularIntensity*specularColor;

    /*
        RIM
    */
    float rim = 0.0;
#ifdef RIM 
    rim = 1.0 - dot(viewDir, normal);
    rim *= pow(nDotL, 0.25);
    float rthreshold = 0.65;
    float rstepsize = 0.01;
    rim = smoothstep(rthreshold, rthreshold+rstepsize, rim);
#endif
    vec3 rimResult = rim*rimIntensity*rimColor;

    /*
        FINAL RESULT
    */
    fragColor.rgb *= (ambientLight + diffuseResult + specularResult + rimResult);
    fragColor.a = 0.0;
    #include code/SetFragment3DOutputs.glsl
};