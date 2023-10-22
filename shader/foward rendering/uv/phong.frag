#version 460

#define USING_VERTEX_TEXTURE_UV

vec3 color;

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

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

void main()
{
    vec4 CE = texture(bColor, uv);
    vec4 NRM = texture(bMaterial, uv);

    mSpecular = 0.9;
    mMetallic = 1.0 - NRM.a;
    mRoughness = NRM.b;
    mEmmisive = 1.0 - CE.a;
    color = CE.rgb;
    normalComposed = perturbNormal(normal, viewVector, NRM.xy, uv);
    colorVCorrection = 1.0-pow(rgb2v(color), 5.0);

    viewDir = normalize(_cameraPosition - position);

    // Calculating skybox reflection
    #ifdef SKYBOX_REFLECTION
        vec3 reflectDir = reflect(viewDir, normalComposed); 
        vec2 uvSky = vec2(0.0);
        uvSky.x = 0.5 + atan(reflectDir.z, -reflectDir.x) / (2.0*PI);
        uvSky.y = -reflectDir.y*0.5 + 0.5;
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

    fragEmmisive = 2.0 * fragColor.rgb *(rgb2v(fragColor.rgb) - ambientLight*0.5);

    // vec3 viewSpaceVector = (inverse(_cameraViewMatrix) * vec4(position, 1.0)).rgb;
    // vec3 viewSpaceVector = transpose(inverse(mat3(_cameraViewMatrix)))*position;
    // fragNormal = perturbNormal(viewNormal, viewSpaceVector, NRM.xy, uv);

    // fragNormal = transpose(inverse(mat3(_cameraViewMatrix)))*normalComposed;

    // fragNormal = normalComposed*0.5 + 0.5;

    // fragNormal = (vec4(normalComposed, 0.0) * inverse(_cameraViewMatrix)).rgb;
    fragNormal = (vec4(normalComposed, 0.0) * inverse(_cameraViewMatrix)).rgb;
    fragNormal = normalize(fragNormal);
    fragNormal = fragNormal*0.5 + 0.5;

    fragNormal = normalComposed*0.5 + 0.5;

    NRM.xy = NRM.xy * 2.0 - 1.0;
    fragNormal = vec3(NRM.xy, sqrt( 1. - dot( NRM.xy, NRM.xy ) ))*0.5 + 0.5;
}
