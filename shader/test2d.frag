#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;

in vec4 vertexColor;
in vec2 quadUv;

out vec4 _fragColor;


void main()
{
    _fragColor = vertexColor;
}