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

out float _AO;


vec3 calculateViewPosition(vec2 textureCoordinate, float depth)
{
    vec4 clipSpacePos = vec4(18.f*(textureCoordinate * 1.0 - 0.5), depth, 1.0);
    vec4 position = clipSpacePos * inverse(_cameraProjectionMatrix);
    position.z = 1.0 - position.z;
    position.xyz /= -position.w;
    position.z = -abs(position.z);
    return position.xyz;
}

// tile noise texture over screen, based on screen dimensions divided by noise size
vec2 noiseScale = vec2(float(iResolution.x)/4.0, float(iResolution.y)/4.0);

int kernelSize = 16; // 64
float radius = 20.0; // 5.0
float bias = 0.1; // 0.5

void main()
{
    vec3 fragPos = calculateViewPosition(uvScreen, texture(bDepth, uvScreen).x);

    vec3 fragColor = texture(bColor, uvScreen).rgb;

    vec3 normal = texture(gNormal, uvScreen).rgb * 2.0 - 1.0;
    if(normal.x == normal.y && normal.y == normal.z && normal.y == 1.0) discard;
    normal = normalize(normal);

    vec3 randomVec = texture(texNoise, uvScreen * noiseScale).xyz * 2.0 - 1.0;
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec2 test = vec2(0.0);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 1.f;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = _cameraProjectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0        

        // get sample depth
        float sampleDepth = calculateViewPosition(offset.xy, texture(bDepth, offset.xy).x).z;
        float rangeCheck = smoothstep(radius, 0.0, abs(fragPos.z - sampleDepth));

        occlusion += (-sampleDepth <= -samplePos.z - bias ? 1.0 : 0.0) * rangeCheck;
    }   

    _AO = (occlusion/float(kernelSize));
}