#version 460

#define USING_VERTEX_TEXTURE_UV

vec3 color;

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (location = 32) uniform vec3 _mageColor;
layout (location = 33) uniform float _mageHP;
layout (location = 34) uniform int _mageType;

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bMaterial;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl

in vec3 viewPos;
in vec3 viewVector;

#define PI 3.1415926538
#define SKYBOX_REFLECTION

////////////////http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{ 
    // get edge vectors of the pixel triangle 
    vec3 dp1 = dFdx( p ); 
    vec3 dp2 = dFdy( p ); 
    vec2 duv1 = dFdx( uv ); 
    vec2 duv2 = dFdy( uv );   
    // solve the linear system 
    vec3 dp2perp = cross( dp2, N ); 
    vec3 dp1perp = cross( N, dp1 ); 
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x; 
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;   
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) ); 
    return mat3( T * invmax, B * invmax, N );
}

#define WITH_NORMALMAP_GREEN_UP

vec3 perturbNormal( vec3 N, vec3 V, vec2 tNormal, vec2 texcoord) 
{ 
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye) 
    vec3 map = vec3(tNormal, 0.0); 
    map = map * 255./127. - 128./127.; 
    map.z = sqrt( 1. - dot( map.xy, map.xy ) ); 

    #ifdef WITH_NORMALMAP_GREEN_UP 
    map.y = -map.y; 
    #endif 

    mat3 TBN = cotangent_frame( N, -V, texcoord ); 
    return normalize( TBN * map );
}
////////////////////////////////////////////////////////////

// vec3 rgb2hsv(vec3 c)
// {
//     vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
//     vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
//     vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

//     float d = q.x - min(q.w, q.y);
//     float e = 1.0e-10;
//     return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
// }

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 CE = texture(bColor, uv);
    vec4 NRM = texture(bMaterial, uv);

    mSpecular = 0.9;

    // mMetallic = 1.0 - NRM.a;
    // mRoughness = NRM.b;

    // mMetallic = float(_mageType)/5.0;

    // mEmmisive = 1.0 - CE.a;
    // color = CE.rgb;
    vec3 mageColor = _mageColor;

    mageColor = rgb2hsv(mageColor);
    mageColor.g += _mageType == 2 ? 0.5 : 0.0;
    mageColor.b += _mageType == 2 ? 0.3 : 0.0;

    mageColor.b += _mageType == 4 ? -0.5 : 0.0;

    mageColor = clamp(mageColor, vec3(0.0), vec3(1.0));

    mageColor = hsv2rgb(mageColor);

    color = mageColor;

    // mEmmisive = 0.35*(1.0 - rgb2v(color));
    normalComposed = perturbNormal(normal, viewVector, NRM.xy, uv);

    colorVCorrection = 1.0-pow(rgb2v(color), 5.0);

    viewDir = normalize(_cameraPosition - position);

    // Calculating skybox reflection
    #ifdef SKYBOX_REFLECTION
        vec3 reflectDir = reflect(viewDir, normalComposed); 
        vec2 uvSky = vec2(0.0);
        uvSky.x = 0.5 + atan(reflectDir.z, -reflectDir.x) / (2.0*PI);
        uvSky.y = reflectDir.y*0.5 + 0.5;
        vec3 rColor = (1.0 - mRoughness)*texture(bSkyTexture, uvSky).rgb; 
    #endif

    Material material = getMultiLightStandard();
    vec3 materialColor = ambientLight + material.diffuse + material.specular + material.fresnel;

    #ifdef SKYBOX_REFLECTION
        fragColor.rgb = mix(color, rColor, (1.0-mRoughness)*0.25)*materialColor;        
        fragColor.rgb *= mix(vec3(1.0), rColor, max(mMetallic*0.9, 0.0));
    #else
         fragColor.rgb = color*materialColor;
    #endif
    
    fragColor.rgb = mix(fragColor.rgb, color / pow(ambientLight+0.5, vec3(0.3)), mEmmisive);

    fragEmmisive = (mEmmisive*25.0) * 2.0 * fragColor.rgb *(rgb2v(fragColor.rgb) - ambientLight*0.5);
    fragEmmisive += 0.5 * fragColor.rgb * (rgb2v(material.specular) - ambientLight);

    fragNormal = normalize((vec4(normal, 0.0) * inverse(_cameraViewMatrix)).rgb)*0.5 + 0.5;
}