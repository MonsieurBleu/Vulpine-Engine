#include functions/HSV.glsl

#include globals/Constants.glsl

#define DIFFUSE
#define SPECULAR
#define FRESNEL

#define BLINN

//////
float mSpecular = 0.5;
float mRoughness = 0.5;
float mMetallic = 0.4;
float mEmmisive = 0.0;
//////

vec3 normalComposed;
vec3 viewDir;
vec3 ambientLight = vec3(0.2);
// vec3 ambientLight = vec3(1.0);
float colorVCorrection;

struct Material
{
    vec3 diffuse;
    vec3 specular;
    vec3 fresnel;
};

Material getDSF(vec3 lightDirection, vec3 lightColor)
{
    float diffuseIntensity = 1.0;
    float specularIntensity = 2.0*colorVCorrection + mMetallic*5.0;
    
    float fresnelIntensity = 0.5 + 2.0*mMetallic;

    // fresnelIntensity *= 2.0;

    vec3 diffuseColor = lightColor;
    vec3 specularColor = lightColor;
    vec3 fresnelColor = lightColor;
    // vec3 fresnelColor = mix(lightColor, vec3(0.075, 0.25, 1.0), 0.75);

    /*
        UTILS
    */
    vec3 nNormal = normalize(normalComposed);
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
    // diffuse = 1.0; // tmp for shadow testing

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

    #ifdef BLINN
        float specularExponent = 36.0 - 32.0*pow(mRoughness, 0.5);
        // specularExponent *= 10.0;
        specular = pow(max(dot(normal, halfwayDir), 0.0), specularExponent);
        specular *= diffuse;
    #else
        float specularExponent = 9.0 - 8.0*pow(mRoughness, 0.5);
        vec3 reflectDir = reflect(lightDirection, nNormal); 
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
    fresnel = (1.0 - dot(normal, viewDir));
    fresnel = pow(fresnel, 4.0);
    fresnel *= diffuse;

    #ifdef TOON
        float rstep = 0.75;
        float rsmooth = 0.05;
        fresnel = smoothstep(rstep, rstep+rsmooth, fresnel);
    #endif

#endif
    vec3 fresnelResult = fresnel*fresnelIntensity*fresnelColor;

    Material result;
    result.diffuse = diffuseResult;
    result.specular = specularResult;
    result.fresnel = fresnelResult;
    return result;
}

// https://www.shadertoy.com/view/wtsSW4
// #define PHI 1.61803398874989484820459 // Golden Ratio   
// #define PI_  3.14159265358979323846264 // PI
// #define SQ2 1.41421356237309504880169 // Square Root of Two
// #define E   2.71828182846
float gold_noise3(in vec3 coordinate, in float seed){
    return 0.5 - fract(tan(distance(coordinate*(seed+PHI*00000.1), vec3(PHI*00000.1, PI*00000.1, E)))*SQR2*10000.0);
}

vec2 goldNoiseCustom(in vec3 coordinate, in float seed)
{
    return vec2(
        fract(tan(distance(coordinate.xy*PHI, coordinate.xy)*seed)*coordinate.x),
        fract(tan(distance(coordinate.zx*PHI, coordinate.zx)*seed)*coordinate.z)
        );
}

// https://github.com/tt6746690/computer-graphics-shader-pipeline/blob/master/src/random2.glsl
vec2 random2(vec3 st){
  vec2 S = vec2( dot(st,vec3(127.1,311.7,783.089)),
             dot(st,vec3(269.5,183.3,173.542)) );
  return fract(sin(S)*43758.5453123);
}

/*
    Efficient soft-shadow with percentage-closer filtering

    link : https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-17-efficient-soft-edged-shadows-using
*/
#define EFFICIENT_SMOOTH_SHADOW
float getShadow(sampler2D shadowmap, mat4 rMatrix)
{
    vec4 mapPosition = rMatrix * vec4(position, 1.0);
    mapPosition.xyz /= mapPosition.w;
    mapPosition.xy = mapPosition.xy*0.5 + 0.5;    

    float res = 0.f;
    float bias = 0.00002;
    // float bias = 0.00005;
    float radius = 0.0009;

    #ifdef EFFICIENT_SMOOTH_SHADOW
        int it = 6;
        int itPenumbra = 64;
        int i;

        for(i = 0; i < it; i++)
            res += texture(shadowmap, mapPosition.xy + 2.0*radius*vec2(gold_noise3(position, i), gold_noise3(position, i*0.2))).r - bias < mapPosition.z ? 1.0 : 0.0;

        if(res < float(it) && res > 0.f)
            for(i = 0; i < itPenumbra; i++)
                res += texture(shadowmap, mapPosition.xy + radius*vec2(gold_noise3(position, i), gold_noise3(position, i*0.2))).r - bias < mapPosition.z ? 1.0 : 0.0;
        
        res /= float(i);
    #else
        res = texture(shadowmap, mapPosition.xy).r - bias < mapPosition.z ? 1.0 : 0.0;
    #endif

    return res;
}

Material getMultiLightStandard()
{
    int id = 0;
    Material result;
    while(true)
    {
        Light light = lights[id];
        Material lightResult = {vec3(0.f), vec3(0.f), vec3(0.f)};
        float factor = 1.0;
        float fresnelFactor = 1.0;
        switch(light.stencil.a)
        {
            case 0 :
                return result;
                break;

            case 1 :

                lightResult = getDSF(light.direction.xyz, light.color.rgb);
                factor = light.color.a;
                fresnelFactor = factor;
                // factor *= light.stencil.b%2 == 0 ? 1.f : getShadow(bSunShadowMap, light._rShadowMatrix);
                factor *= light.stencil.b%2 == 0 ? 1.f : getShadow(bShadowMaps[light.stencil.r], light._rShadowMatrix);
                // factor *= light.stencil.b%2 == 0 ? 1.f : getShadow(bShadowMaps[int(_iTime)%2], light._rShadowMatrix);
                break;

            case 2 : 
            {
                float maxDist = max(light.direction.x, 0.0001);
                float distFactor = max(maxDist - distance(position, light.position.xyz), 0.f)/maxDist;
                vec3 direction = normalize(position - light.position.xyz);
                
                lightResult = getDSF(direction, light.color.rgb);
                factor = distFactor*distFactor*light.color.a;
                fresnelFactor = factor;
            }
                break;

            case 3 : 
            {
                vec3 pos1 = light.position.xyz;
                vec3 pos2 = light.direction.xyz;
                vec3 H = position-pos1;
                vec3 tubeDir = normalize(pos1-pos2);
                float cosinus = dot(normalize(H), tubeDir);
                float A = cosinus * length(H);
                vec3 sPos = pos1+tubeDir*A;

                float segL = length(pos1-pos2);
                sPos = mix(sPos, pos1, step(segL, length(sPos-pos2)));
                sPos = mix(sPos, pos2, step(segL, length(sPos-pos1)));

                float radius = 5.0;

                /*
                    TODO : fix radius
                */
                float maxDist = max(light.direction.a, 0.0001);
                float distFactor = max(maxDist - distance(sPos, position), 0.f)/maxDist;
                vec3 direction = normalize(position - sPos);
                
                lightResult = getDSF(direction, light.color.rgb);
                factor = distFactor*distFactor*light.color.a;
                fresnelFactor = factor;
            }
                break;

            default : break;
        }
        
        
        result.diffuse += lightResult.diffuse * factor;
        result.specular += lightResult.specular * factor;
        result.fresnel += lightResult.fresnel * fresnelFactor;

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