#include functions/HSV.glsl

layout (binding = 4) uniform sampler2D bSkyTexture;

#define DIFFUSE
#define SPECULAR
#define FRESNEL

#define BLINN

//////
float mSpecular = 0.5;
float mRoughness = 0.5;
float mMetallic = 0.4;
//////

vec3 normalComposed;
vec3 ambientLight = vec3(0.1);

struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 fresnel;
};


Material getDSF(vec3 lightDirection, vec3 lightColor)
{
    float diffuseIntensity = 0.5;
    // float specularIntensity = 5.0*mSpecular;
    // float specularIntensity = (1.0-rgb2v(color)) + mMetallic*40.0;
    // float specularIntensity = 2.0*(1.0-pow(rgb2v(color), 5.0)) + mMetallic*40.0;
    float specularIntensity = 2.0*(1.0-pow(rgb2v(color), 5.0)) + mMetallic*5.0;
    
    float fresnelIntensity = 2.0 + 1.0*mMetallic;

    vec3 diffuseColor = lightColor;
    vec3 specularColor = lightColor;
    vec3 fresnelColor = lightColor;

    /*
        UTILS
    */
    vec3 nNormal = normalize(normalComposed);

    vec3 viewDir = normalize(_cameraPosition - position);
    vec3 reflectDir = reflect(lightDirection, nNormal); 
    float nDotL = max(dot(-lightDirection, nNormal), 0.f);

#ifdef BLINN
    vec3 halfwayDir = normalize(-lightDirection+viewDir);
#endif

    /*
        DIFFUSE 
    */
    float diffuse = 0.0;
#ifdef DIFFUSE
    // diffuse = pow(nDotL, 0.5);
    diffuse = nDotL;

    #ifdef TOON
        float dstep = 0.1;
        float dsmooth = 0.05;
        diffuse = smoothstep(dstep, dstep+dsmooth, diffuse);
    #endif

#endif
    vec3 diffuseResult = diffuse*diffuseIntensity*diffuseColor;


    /*
        SPECULAR
    */
    float specular = 0.0;
#ifdef SPECULAR
    // float specularExponent = 32.0 - mSpecular*31.0;
    // float specularExponent = 48.0 - mSpecular*32.0;
    float specularExponent = 1.0 + 8.0 - 8.0*pow(mRoughness, 0.1);
    // float specularExponent = 4.0;
    // specular = pow(max(dot(reflectDir, viewDir), 0.0), specularExponent);

    #ifdef BLINN
        specular = pow(max(dot(normal, halfwayDir), 0.0), 4.0*specularExponent);
    #else
        specular = pow(max(dot(reflectDir, viewDir), 0.0), specularExponent);
    #endif

    #ifdef TOON
        float sstep = 0.1;
        float ssmooth = 0.05;
        specular = smoothstep(sstep, sstep+ssmooth, specular);
    #endif
#endif
    vec3 specularResult = specular*specularIntensity*specularColor;

    /*
        FRESNEL
    */
    float fresnel = 0.0;
#ifdef FRESNEL 
    fresnel = 1.0 - dot(normal, viewDir);

    fresnel *= diffuse;
    // fresnel *= smoothstep(0.0, 0.1, diffuse);

    fresnel = pow(fresnel, 2.0);

    #ifdef TOON
        float rstep = 0.75;
        float rsmooth = 0.05;
        fresnel = smoothstep(rstep, rstep+rsmooth, fresnel);
    #endif

#endif
    vec3 fresnelResult = fresnel*fresnelIntensity*fresnelColor;

    // return diffuseResult + specularResult + fresnelResult;
    // return max(diffuseResult, fresnelResult) + specularResult;
    // return diffuseResult + specularResult + fresnelResult;

    Material result;
    result.diffuse = diffuseResult;
    result.specular = specularResult;
    result.fresnel = fresnelResult;
    return result;
}

Material getMultiLightStandard()
{
    int id = 0;
    // vec3 result = vec3(0.0);
    Material result;
    while(true)
    {
        Light light = lights[id];
        Material lightResult = {vec3(0.f), vec3(0.f), vec3(0.f)};
        float factor = 1.0;
        switch(light.stencil.a)
        {
            case 0 :
                return result;
                break;

            case 1 :

                lightResult = getDSF(light.direction.xyz, light.color.rgb);
                factor = light.color.a;
                break;

            case 2 : 
            {
                float maxDist = max(light.direction.x, 0.0001);
                float distFactor = max(maxDist - distance(position, light.position.xyz), 0.f)/maxDist;
                // distFactor = pow(distFactor, 2.0);
                vec3 direction = normalize(position - light.position.xyz);
                // vec3 direction = vec3(1.0);
                // float finalFactor = max(distFactor*light.color.a - ambientLight.x*3.0, 0.f);
                
                lightResult = getDSF(direction, light.color.rgb);
                factor = distFactor*light.color.a;
            }
                break;

            default : break;
        }
        
        
        result.diffuse += lightResult.diffuse * factor;
        result.specular += lightResult.specular * factor;
        result.fresnel += lightResult.fresnel * factor;

        // result += max(lightResult - ambientLight*0.5, vec3(0.f));

        id ++;
    }

    return result;
}

vec3 getStandardEmmisive(vec3 fcolor, vec3 ambientLight)
{
    // return  color.rgb * 
    //         max(
    //             rgb2v(color.rgb) - min(max(ambientLight*1.5, 0.1), 0.8), 
    //             0.0);

    return fcolor * (rgb2v(fcolor) - ambientLight);
}