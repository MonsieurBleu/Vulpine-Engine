#ifndef FNCT_SKYBOX_GLSL
#define FNCT_SKYBOX_GLSL

#ifdef CUBEMAP_SKYBOX
    layout (binding = 4) uniform samplerCube bSkyTexture; 
#else
    layout (binding = 4) uniform sampler2D bSkyTexture;
#endif


// Reinhard tone mapping
vec3 toneMapReinhard(vec3 color, float exposure, float whitePoint) {
    color = color * exposure;
    return color / (color + vec3(1.0)) * whitePoint;
}

vec3 getSkyColor(vec2 uv)
{
    vec3 c = texture(bSkyTexture, uv).rgb;

    float exposure = 0.5;
    float gamma = 2.0;

    // float brightMax = 0.1;
    // exposure = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0);
    
    // c = vec3(1.0) - exp(-c*exposure);

    // c *= pow(2.0, exposure);
    // c = toneMapReinhard(c, exposure, 0.1);

    // c *= exposure;

    // c = pow(c, vec3(1.0/gamma));

    return c;
}

#endif
