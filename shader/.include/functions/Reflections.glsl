#ifndef FNCT_REFLECTIONS_GLSL
#define FNCT_REFLECTIONS_GLSL

#include functions/Skybox.glsl

float getReflectionFactor(float fresnel, float metallic, float roughness)
{
    const float maxReflectivity = 0.6;
    const float metallicExponent = 3.0*(1.0 - metallic);
    const float roughnessFactor = 1.0-roughness;
    const float reflectFactor = roughnessFactor*min(maxReflectivity, pow(fresnel, metallicExponent));
    return reflectFactor;
}

vec3 getSkyboxReflection(vec3 v, vec3 n)
{
    #ifdef SKYBOX_REFLECTION
        vec3 reflectDir = normalize(reflect(v, n));
        #ifdef CUBEMAP_SKYBOX
            reflectColor = texture(bSkyTexture, -reflectDir).rgb;
        #else

            vec2 uvSky = vec2(
                0.5 + atan(reflectDir.z, -reflectDir.x)/(2.0*PI), 
                reflectDir.y*0.5 + 0.5
            );

            /*
                Fast roughness reflection blur
            */
            const float noiseMaxDist = 0.05;
            vec3 noiseUv = vec3(uv, 1.0);
            uvSky += (1.0 - 2.0*random2(noiseUv))*noiseMaxDist*mRoughness;
            uvSky.y = clamp(uvSky.y, 0.f, 0.999);

            return getSkyColor(uvSky); 
        #endif
    #else
        return vec3(0.f);
    #endif
}

#endif
