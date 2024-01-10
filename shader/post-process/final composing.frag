#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;

layout (location = 10) uniform int bloomEnable;

layout (binding = 0) uniform sampler2D bColor;
layout (binding = 1) uniform sampler2D bDepth;
layout (binding = 2) uniform sampler2D bNormal;
layout (binding = 3) uniform sampler2D bAO;
layout (binding = 4) uniform sampler2D bEmmisive;
layout (binding = 5) uniform sampler2D texNoise;
layout (binding = 6) uniform sampler2D bSunMap;
layout (binding = 7) uniform sampler2D bUI;

in vec2 uvScreen;
in vec2 ViewRay;

out vec4 _fragColor;

// #define SHOW_SHADOWMAP

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float LinearizeDepth(float depth, float zNear, float zFar)
{
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

vec4 getBlurAO(vec2 TexCoords)
{
    vec2 texelSize = 1.0 / vec2(textureSize(bAO, 0));
    vec4 result = vec4(0.0);
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(bAO, TexCoords + offset);
        }
    }
    return result / (4.0 * 4.0);
}

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  vec2 off4 = vec2(4.123489131235294) * direction;
  color += texture(image, uv) * 0.1964825501511404;
  color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
  color += texture(image, uv + (off4 / resolution)) * 0.214563213624011544;
  color += texture(image, uv - (off4 / resolution)) * 0.214563213624011544;
  return color;
}


bool horizontal = false;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

vec3 getBloom(vec2 TexCoords)
{             
    vec2 tex_offset = 1.0 / textureSize(bEmmisive, 0); // gets size of single texel
    vec3 result = texture(bEmmisive, TexCoords).rgb * weight[0]; // current fragment's contribution
    
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(bEmmisive, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(bEmmisive, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(bEmmisive, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(bEmmisive, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    
    return result;
}

// How far from the center to take samples from the fragment you are currently on
const int radius = 25;
// Keep it between 1.0f and 2.0f (the higher this is the further the blur reaches)
float spreadBlur = 2.f;
float weights[radius];

vec3 blur(sampler2D screenTexture, vec2 texCoords)
{             
    // Calculate the weights using the Gaussian equation
    float x = 0.0f;
    for (int i = 0; i < radius; i++)
    {
        // Decides the distance between each sample on the Gaussian function
        if (spreadBlur <= 2.0f)
            x += 3.0f / radius;
        else
            x += 6.0f / radius;

        weights[i] = exp(-0.5f * pow(x / spreadBlur, 2.0f)) / (spreadBlur * sqrt(2 * 3.14159265f));
    }


    vec2 tex_offset = 1.0f / textureSize(screenTexture, 0);
    vec3 result = texture(screenTexture, texCoords).rgb * weights[0];

    // Calculate horizontal blur
    // if(horizontal)
    // {
        for(int i = 1; i < radius; i+= 5)
        {
            // Take into account pixels to the right
            result += texture(screenTexture, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weights[i];
            // Take into account pixels on the left
            result += texture(screenTexture, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weights[i];
        }
    // }
    // Calculate vertical blur
    // else
    // {
        for(int i = 1; i < radius; i+= 5)
        {
            // Take into account pixels above
            result += texture(screenTexture, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weights[i];
            // Take into account pixels below
            result += texture(screenTexture, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weights[i];
        }
    // }
    
    return result;
}

void main()
{
    vec2 uv = uvScreen;
    float aspectRatio = float(iResolution.y)/float(iResolution.x);

    // Pixel art effect 
        // float pixelSize = 0.0075;

        // /* Additionnal depth based resolution change */
        // float d = texture(bDepth, uv).r*2.0;
        // d = min(d, 0.05);
        // d = d - mod(d, 0.005);
        // pixelSize = 300.0 * pixelSize * (0.001 + pow(d, 2.0));

        // uv = uv * vec2(1.0, aspectRatio);
        // uv = uv - mod(uv, vec2(pixelSize)) + pixelSize*0.5;
        // uv = uv / vec2(1.0, aspectRatio);
    //////////////////

    vec3 BackgroundColor = vec3(0.2, 0.3, 0.3);
    vec4 color = texture(bColor, uv);
    // _fragColor.rgb = mix(BackgroundColor, color.rgb, color.a);

    float chromaticAberation = 0.0 / float(iResolution.y);
    vec2 rUv = max(uv - chromaticAberation, 0.0);
    vec2 gUv = min(uv + chromaticAberation, 1.0);
    vec2 bUv = uv;
    _fragColor.r = texture(bColor, rUv).r;
    _fragColor.g = texture(bColor, gUv).g;
    _fragColor.b = texture(bColor, bUv).b;

    vec4 AO = getBlurAO(uv);
    _fragColor.rgb *= vec3(1.0 - AO.r);
    // _fragColor.rgb *= vec3(1.0) - AO.rgb;

    // vec3 bloom = blur(bEmmisive, uvScreen);
    // _fragColor.rgb += bloom;
    // _fragColor.rgb += texture(bNormal, uvScreen).rgb;
    // _fragColor.rgb = 1.0 - texture(bAO, uvScreen).rgb;

    if(bloomEnable != 0) 
        // _fragColor.rgb += 0.125*texture(bEmmisive, uv).rgb; 
        _fragColor.rgb += 0.25*texture(bEmmisive, uv).rgb; 


    
    // float exposure = 1.0;
    // float gamma = 1.75;

    // float exposure = 1.0;
    // float gamma = 2.75;

    float exposure = 1.0;
    float gamma = 2.2;

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-_fragColor.rgb * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
    _fragColor.rgb = mapped;

    
    /// DEPTH 
        // float depth = texture(bDepth, uvScreen).r;
        // _fragColor.rgb = vec3(1.0 - pow(1.0 - depth, 50.0));
    ////////

    #ifdef SHOW_SHADOWMAP
        vec2 SSMuv = uvScreen * vec2(iResolution) * 1/900.0;
        if(SSMuv.x >= 0.f && SSMuv.x <= 1.0 && SSMuv.y >= 0.f && SSMuv.y <= 1.0)
        {
            // float d = texture(bSunMap, SSMuv).r;
            // d = pow(d, 100.0)*5000000000.0;
            // _fragColor.rgb = vec3(d);
            _fragColor.rgb = texture(bSunMap, SSMuv).rgb;
        }
    #endif

    //// REPERE
        // _fragColor.rgb = mix(_fragColor.rgb, vec3(0.0, 1.0, 0.0), step(abs(uvScreen.x-0.5), 0.0005));
        // _fragColor.rgb = mix(_fragColor.rgb, vec3(0.0, 1.0, 0.0), step(abs(uvScreen.y-0.5), 0.001));
    ////

    //// DEPTH BASED FOG
        // float d = texture(bDepth, uv).r*2.0;
        // float base = 0.001;
        // d = smoothstep(base + 0.05, base, d)*0.99;
        // _fragColor.rgb = mix(_fragColor.rgb, vec3(0.85), d);
    ////


    vec4 ui = texture(bUI, uvScreen);
    _fragColor.rgb = mix(_fragColor.rgb, ui.rgb, ui.a);
    _fragColor.a = 1.0;

    // _fragColor.rgb = vec3(1.0 - AO.r);
    // _fragColor.rgb = 1.0 - AO.rgb;
}