#include functions/HSV.glsl

#include globals/Constants.glsl
#include functions/Noise.glsl

float mRoughness;
float mRoughness2;
float mMetallic;
float mEmmisive;

// vec3 ambientLight = vec3(0.2);
vec3 normalComposed;
vec3 viewDir;
vec3 color;
float nDotV;

struct Material
{
    vec3 result;
    float fresnel;
};

Material getBRDF(vec3 lightDirection, vec3 lightColor)
{
    vec3 F0 = mix(vec3(0.04), color, mMetallic);

    vec3 halfwayDir = normalize(-lightDirection+viewDir);
    float nDotH = max(dot(normalComposed, halfwayDir), 0.0);
    float nDotH2 = nDotH*nDotH;
    float nDotL = max(dot(normalComposed, -lightDirection), 0.0);

    vec3 fresnelSchlick = F0 + (1.0 - F0) * pow(1.0 - nDotH, 5.0);

    float nDenom = (nDotH2 * (mRoughness2 - 1.0) + 1.0);
    float normalDistrib = mRoughness2 / (PI * nDenom * nDenom);

    float gK = mRoughness2*0.5;
    float gKi = 1.0 - gK;
    float geometry = (nDotL*nDotV)/((nDotV*gKi + gK)*(nDotL*gKi + gK));

    vec3 specular = fresnelSchlick*normalDistrib*geometry/max((4.f*nDotV*nDotL), 0.00000000001);

    vec3 kD = (vec3(1.0) - fresnelSchlick)*(1.0 - mMetallic);
    vec3 diffuse = kD*color/PI;

    Material result;
    result.result = (specular + diffuse) * lightColor * nDotL * 2.0;

    return result;
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
    float radius = 0.0015;

    #ifdef EFFICIENT_SMOOTH_SHADOW
        int it = 8;
        int itPenumbra = it + 32;
        int i = 0;

        for(; i < it; i++)
            res += texture(
                shadowmap, 
                mapPosition.xy + 2.0*radius*vec2(gold_noise3(position, i), 
                gold_noise3(position, i*0.2))).r 
            - bias < mapPosition.z ? 1.0 : 0.0;

        if(res < float(it) && res > 0.f)
        {
            float p = float(it)*0.5;
            float prct = 0.5 + 0.5*abs(res-p)/p;
            itPenumbra = int(float(itPenumbra)*prct);

            for(; i < itPenumbra; i++)
                res += texture(
                    shadowmap, 
                    mapPosition.xy + radius*vec2(gold_noise3(position, i), 
                    gold_noise3(position, i*0.2))).r 
                - bias < mapPosition.z ? 1.0 : 0.0;
        }
        
        res /= float(i+1);
    #else
        res = texture(shadowmap, mapPosition.xy).r - bias < mapPosition.z ? 1.0 : 0.0;
    #endif

    return res;
}

Material getMultiLightPBR()
{
    int id = 0;
    Material result;

    nDotV = max(dot(normalComposed, viewDir), 0.0);
    result.fresnel = 1.0 - nDotV;
    
    while(true)
    {
        Light light = lights[id];
        Material lightResult = {vec3(0.f), 0.f};
        float factor = 1.0;
        switch(light.stencil.a)
        {
            case 0 :
                return result;
                break;

            case 1 :

                lightResult = getBRDF(light.direction.xyz, light.color.rgb);
                factor = light.color.a;
                factor *= 
                    light.stencil.b%2 == 0 ? 
                        1.f : 
                        getShadow(bShadowMaps[light.stencil.r], light._rShadowMatrix);
                break;

            case 2 : 
            {
                float maxDist = max(light.direction.x, 0.0001);
                float distFactor = max(maxDist - distance(position, light.position.xyz), 0.f)/maxDist;
                vec3 direction = normalize(position - light.position.xyz);
                
                lightResult = getBRDF(direction, light.color.rgb);
                factor = distFactor*distFactor*light.color.a;
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
                
                lightResult = getBRDF(direction, light.color.rgb);
                factor = distFactor*distFactor*light.color.a;
            }
                break;

            default : break;
        }
        
        result.result += lightResult.result * factor;

        id ++;
    }

    return result;
}

vec3 getStandardEmmisive(vec3 fcolor)
{
    vec3 baseEmmissive = fcolor*(rgb2v(fcolor) - ambientLight*0.5);
    vec3 finalEmmisive = mix(baseEmmissive, 5.0*fcolor, mEmmisive);
    return finalEmmisive;
}