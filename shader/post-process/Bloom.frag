#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 3) uniform int pass;

layout (binding = 4) uniform sampler2D bSource;

in vec2 uvScreen;

out vec3 fragColor;

#include functions/HSV.glsl

// How far from the center to take samples from the fragment you are currently on
const int radius = 70;
// Keep it between 1.0f and 2.0f (the higher this is the further the blur reaches)
float spreadBlur = 2.0;
float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
bool horizontal = true;

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
    if(pass == 1)
    {
        for(int i = 1; i < radius; i+= 5)
        {
            // Take into account pixels to the right
            result += texture(screenTexture, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weights[i];
            // Take into account pixels on the left
            result += texture(screenTexture, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weights[i];
        }
    }
    // Calculate vertical blur
    else
    {
        for(int i = 1; i < radius; i+= 5)
        {
            // Take into account pixels above
            result += texture(screenTexture, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weights[i];
            // Take into account pixels below
            result += texture(screenTexture, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weights[i];
        }
    }
    
    return result;
}


// https://github.com/Jam3/glsl-fast-gaussian-blur
vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture(image, uv) * 0.1964825501511404;
  color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}

float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
vec3 blur2(sampler2D image)
{
    vec4 FragmentColor;

    FragmentColor = texture(image, vec2(gl_FragCoord) / iResolution) * weight[0];
    for (int i=1; i<3; i++) {
        FragmentColor +=
            texture(image, (vec2(gl_FragCoord) + vec2(0.0, offset[i])) / iResolution)
                * weight[i];
        FragmentColor +=
            texture(image, (vec2(gl_FragCoord) - vec2(0.0, offset[i])) / iResolution)
                * weight[i];
    }
    return FragmentColor.rgb;
}

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

void main()
{
    // fragColor = texture(bSource, uvScreen).rgb;

    // fragColor = blur2(bSource);

    float b = 0.4;

    // if(pass == 2) b = 0.25;

    fragColor = b * blur(bSource, uvScreen).rgb;

    if(pass == 1)
    {
        // fragColor *= 1.0 - 1.5*rgb2v(texture(bSource, uvScreen).rgb);
    }

    // vec2 direction = vec2(10.0);

    // if(pass == 1) direction *= vec2(0.2, -0.75);
    // if(pass == 2) direction *= vec2(-0.96, 0.38);

    // fragColor = 2.0 * blur13(bSource, uvScreen, vec2(iResolution), direction).rgb;
}