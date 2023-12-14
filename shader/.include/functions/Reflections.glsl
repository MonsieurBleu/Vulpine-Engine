#ifndef FNCT_REFLECTIONS_GLSL
#define FNCT_REFLECTIONS_GLSL

#ifdef CUBEMAP_SKYBOX
    layout (binding = 4) uniform samplerCube bSkyTexture; 
#else
    layout (binding = 4) uniform sampler2D bSkyTexture;
#endif

vec3 getSkyboxReflection(vec3 v, vec3 n)
{
    #ifdef SKYBOX_REFLECTION
        vec3 reflectDir = normalize(reflect(v, n));
        #ifdef CUBEMAP_SKYBOX
            reflectColor = (1.0 - mRoughness)*texture(bSkyTexture, -reflectDir).rgb;
        #else

            vec2 uvSky = vec2(
                0.5 + atan(reflectDir.z, -reflectDir.x)/(2.0*PI), 
                reflectDir.y*0.5 + 0.5
            );

            /*
                Fast roughness reflection blur
            */
            uvSky += (1.0 - 2.0*random2(vec3(uv, 1.0)))*0.035*(mRoughness);
            uvSky.y = clamp(uvSky.y, 0.f, 0.999);

            const float baseReflect = 0.5;
            float reflectionFactor = baseReflect + (1.0-baseReflect)*(1.0 - mRoughness);
            reflectionFactor = 0.35 * pow(reflectionFactor, 7.0);

            return reflectionFactor*texture(bSkyTexture, uvSky).rgb; 
        #endif
    #elif
        return vec3(0.f);
    #endif
}

#endif
