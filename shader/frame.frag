#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;

layout (location = 0) in vec2 vertexPosition;

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bDepth;
layout (binding = 2) uniform sampler2D bNormal;
layout (binding = 3) uniform sampler2D bAlbedo;
layout (binding = 4) uniform sampler2D bPositon;

layout (binding = 5) uniform sampler2D texNoise;
layout (binding = 6) uniform sampler2D bAO;

in vec2 uvScreen;
in vec2 ViewRay;

out vec4 _fragColor;

float LinearizeDepth(in vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 1000.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture(bDepth, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(vec4 sRGB)
{
    return sRGB;

    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
    vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb/vec3(12.92);

    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

vec3 getBlurAO(vec2 TexCoords)
{
    vec2 texelSize = 1.0 / vec2(textureSize(bAO, 0));
    vec3 result = vec3(0.0);
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(bAO, TexCoords + offset).rgb;
        }
    }
    return result / (4.0 * 4.0);
}

void main()
{
    vec3 BackgroundColor = vec3(0.2, 0.3, 0.3);
    vec4 color = texture(bColor, uvScreen);
    // _fragColor.rgb = mix(BackgroundColor, color.rgb, color.a);

    float chromaticAberation = 0.0 / float(iResolution.y);
    vec2 rUv = max(uvScreen - chromaticAberation, 0.0);
    vec2 gUv = min(uvScreen + chromaticAberation, 1.0);
    vec2 bUv = uvScreen ;
    _fragColor.r = texture(bColor, rUv).r;
    _fragColor.g = texture(bColor, gUv).g;
    _fragColor.b = texture(bColor, bUv).b;

    _fragColor.rgb *= getBlurAO(uvScreen);

    _fragColor.a = 1.0;
}