#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 9) uniform mat4 _cameraProjectionMatrix; 

layout (location = 16) uniform vec3 samples[64];

layout (location = 0) in vec2 vertexPosition;

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bDepth;
layout (binding = 2) uniform sampler2D gNormal;
layout (binding = 3) uniform sampler2D texNoise;

in vec2 uvScreen;

out vec3 _AO;

vec3 calculate_view_position(vec2 texture_coordinate, float depth_from_depth_buffer)
{
    vec3 clip_space_position = vec3(texture_coordinate, depth_from_depth_buffer) * 2.0 - vec3(1.0);

    mat4 inverse_projection_matrix = inverse(_cameraProjectionMatrix);

    vec4 view_position =
        vec4(
            vec2(inverse_projection_matrix[0][0], inverse_projection_matrix[1][1]) * clip_space_position.xy,
            -1,
            inverse_projection_matrix[2][3] * clip_space_position.z + inverse_projection_matrix[3][3]);

    return(view_position.xyz / view_position.w);
}

// tile noise texture over screen, based on screen dimensions divided by noise size
vec2 noiseScale = vec2(float(iResolution.x)/4.0, float(iResolution.y)/4.0);

int kernelSize = 64;
float radius = 5.0;
float bias = 1.0;

void main()
{
    vec3 fragPos = calculate_view_position(uvScreen, texture(bDepth, uvScreen).x);

    vec3 normal = texture(gNormal, uvScreen).rgb * 2.0 - 1.0;
    vec3 randomVec = normalize(texture(texNoise, uvScreen * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec2 test = vec2(0.0);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = _cameraProjectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0        

        // get sample depth
        float sampleDepth = calculate_view_position(offset.xy, texture(bDepth, offset.xy).x).z;

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;     
    }  
    occlusion = 1.0 - (occlusion / kernelSize);

    _AO = vec3(pow(occlusion, 2.0));
}