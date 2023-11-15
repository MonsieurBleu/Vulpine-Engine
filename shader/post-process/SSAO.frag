#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 3) uniform mat4 _cameraViewInverse;
layout (location = 4) uniform mat4 _cameraViewMatrix;
layout (location = 6) uniform vec3 _cameraDirection; 
layout (location = 9) uniform mat4 _cameraProjectionMatrix; 

layout (location = 16) uniform vec3 samples[64];

layout (location = 0) in vec2 vertexPosition;

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bDepth;
layout (binding = 2) uniform sampler2D gNormal;
layout (binding = 3) uniform sampler2D texNoise;

layout (binding = 2) uniform sampler2D bNormal;

in vec2 uvScreen;

out vec4 _AO;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 calculateViewPosition(vec2 textureCoordinate, float depth)
{
    // return _cameraDirection * (1.0-depth);

    float test = 18.0;

    vec4 clipSpacePos = vec4(test*(textureCoordinate * 1.0 - 0.5), 
                            depth,
                            1.0);
    
    vec4 position = clipSpacePos * inverse(_cameraProjectionMatrix);

    position.z = 1.0 - position.z;

    // position.z *= 0.05;
    
    position.xyz /= -position.w;

    position.z = -abs(position.z);

    return position.xyz;
}

// tile noise texture over screen, based on screen dimensions divided by noise size
vec2 noiseScale = vec2(float(iResolution.x)/4.0, float(iResolution.y)/4.0);

int kernelSize = 16; // 64
float radius = 20.0; // 5.0
float bias = 0.1; // 0.5
float colorBias = 0.1;

void main()
{
    vec3 fragPos = calculateViewPosition(uvScreen, texture(bDepth, uvScreen).x);
    // vec3 fragWorldPos = (_cameraViewInverse * vec4(fragPos, 1.0)).xyz;

    vec3 fragColor = texture(bColor, uvScreen).rgb;

    vec3 normal = texture(gNormal, uvScreen).rgb * 2.0 - 1.0;
    if(normal.x == normal.y && normal.y == normal.z && normal.y == 1.0) discard;
    // normal = (_cameraViewInverse * vec4(normal, 0.0)).rgb; 
    normal = normalize(normal);

    vec3 randomVec = texture(texNoise, uvScreen * noiseScale).xyz * 2.0 - 1.0;
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec2 test = vec2(0.0);

    // iterate over the sample kernel and calculate occlusion factor
    vec4 occlusion = vec4(0.0);
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        // vec3 samplePos = TBN * normalize(samples[i]); // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        // samplePos = fragWorldPos + samplePos * radius; 
        // samplePos = (_cameraViewMatrix * vec4(samplePos, 1.0)).xyz;
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = _cameraProjectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0        
        // offset.z = 1.0 - offset.z;

        // get sample depth
        float sampleDepth = calculateViewPosition(offset.xy, texture(bDepth, offset.xy).x).z;

        // range check & accumulate
        // float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        // float rangeCheck = step(abs(fragPos.z - sampleDepth), radius);
        float rangeCheck = smoothstep(radius, 0.0, abs(fragPos.z - sampleDepth));
        // rangeCheck = 1.0;

        // occlusion += (sampleDepth >= samplePos.z + bias ? vec3(1.0) : vec3(0.0)) * rangeCheck * (1.0 - texture(bColor, offset.xy).rgb);     
        
        occlusion.a += (-sampleDepth <= -samplePos.z - bias ? 1.0 : 0.0) * rangeCheck;  

        vec3 sampleColor = texture(bColor, offset.xy).rgb;
        occlusion.rgb += (-sampleDepth <= -samplePos.z - colorBias ? vec3(1.0 - sampleColor) : vec3(0.0)) * rangeCheck;  



        // if(sampleDepth >= samplePos.z + bias)
        //     occlusion.a += rangeCheck;

        // if(sampleDepth >= samplePos.z + colorBias)
        // {
        //     vec3 sampleColor = texture(bColor, offset.xy).rgb;
        //     occlusion.rgb = mix(
        //         occlusion.rgb, 
        //         1.0 - sampleColor, 
        //         (rangeCheck/kernelSize));
        // }
    }   
    // occlusion.rgb = 1.0 - (occlusion.rgb / kernelSize);
    // occlusion.a /= kernelSize;
    // occlusion.rgb *= pow(2.0, rgb2hsv(occlusion.rgb).b - 1.0);

    float lumDiscriminationExp = 1.15;
    // occlusion.rgb *= 2.0 * pow(rgb2hsv(occlusion.rgb).b, lumDiscriminationExp);
    // occlusion = 1.0 - occlusion;
    // occlusion.rgb *= 5.0 * pow(1.0 - rgb2hsv(occlusion.rgb).b, lumDiscriminationExp);

    // _AO.rgb = 1.0 - pow(1.0 - occlusion.rgb/float(kernelSize), vec3(7.0));
    // _AO.a = 1.0 - pow(1.0 - (occlusion.a/float(kernelSize)), 15.0); //6
    // _AO.a = -fragPos.z > 90000.0 ? 1.0 : _AO.a;

    // if(occlusion == 1.0)
    //     _AO = vec3(0.25, 0.0, 0.0);

    // _AO.rgb = calculateViewPosition(uvScreen, texture(bDepth, uvScreen).x);

    // _AO.a = fragPos.z;

    _AO.rgb = pow((occlusion.rgb/float(kernelSize)), vec3(1.5))*4.0;
    _AO.a = (occlusion.a/float(kernelSize));
}