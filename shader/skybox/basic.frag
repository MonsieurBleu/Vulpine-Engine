#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;

in vec2 uvScreen;
out vec3 fragColor;

void main()
{
    fragColor = vec3(uvScreen, 1.0);
}
