#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;

in vec3 vertexColor;
in vec3 vertexNormal;

out vec4 fragColor;

void main()
{
    fragColor.rgb = vertexColor;

    fragColor.a = 1.0;
};