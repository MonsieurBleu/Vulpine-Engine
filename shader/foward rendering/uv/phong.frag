#version 460

#define USING_VERTEX_TEXTURE_UV

vec3 color;

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl
#include uniform/Ligths.glsl

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bMaterial;
// layout (binding = 2) uniform sampler2D bRoughness;
// layout (binding = 3) uniform sampler2D bMetallic;

#include globals/Fragment3DInputs.glsl
#include globals/Fragment3DOutputs.glsl

#include functions/standardMaterial.glsl

in vec3 viewPos;
in vec3 viewVector;

#define PI 3.1415926538



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
    // mMetallic = max(1.0 - NRM.a, 0.5*(1.0 - NRM.b));
    mRoughness = NRM.b;
    color = CE.rgb;
    normalComposed = perturbNormal(normalize(normal), viewVector, NRM.xy, uv);

    vec3 nNormal = normalize(normalComposed);
    vec3 viewDir = normalize(_cameraPosition - position);
    vec3 reflectDir = reflect(viewDir, nNormal); 

    vec2 uvSky = vec2(0.0);
    uvSky.x = 0.5 + atan(reflectDir.z, -reflectDir.x) / (2.0*PI);
    uvSky.y = -reflectDir.y*0.5 + 0.5;

    vec3 rColor = (1.0 - mRoughness)*texture(bSkyTexture, uvSky).rgb; 
    // vec3 rColor = texture(bSkyTexture, uvSky).rgb;

    

    #include code/SetFragment3DOutputs.glsl
    Material material = getMultiLightStandard();
    vec3 materialColor = ambientLight + material.diffuse + material.specular + material.fresnel;

    // fragColor.rgb = mix(color*materialColor, rColor, max((1.0-mRoughness)*0.125, 0.0));
    
    fragColor.rgb = mix(color, rColor, (1.0-mRoughness)*0.25)*materialColor;        
    // fragColor.rgb = color*materialColor;
    fragColor.rgb *= mix(vec3(1.0), rColor, max(mMetallic*0.9, 0.0));

    fragEmmisive = fragColor.rgb * 2.0 * (rgb2v(fragColor.rgb) - ambientLight*0.5);

    // vec3 e = material.diffuse + material.fresnel + rColor;
    // fragEmmisive = 0.5 * pow(color, vec3(1.0)) * e * (rgb2v(e) - ambientLight*0.5);
    // fragEmmisive = 0.5 * pow(color, vec3(0.5)) * e * (rgb2v(e) - ambientLight*0.5);
    // fragEmmisive = pow(fragEmmisive, vec3(5.0));

    // vec3 e = materialColor + 0.25*rColor;
    // fragEmmisive = 0.5*pow(color, vec3(0.35))*(rgb2v(e) - ambientLight);

    // fragColor.rgb = texture(bMetallic, vec2(uv.x, 1.0 - uv.y)).rgb;

    // fragNormal = perturb_normal(viewNormal, viewVector, vec2(uv.x, 1.0 - uv.y));
    fragNormal = normalComposed;
}
