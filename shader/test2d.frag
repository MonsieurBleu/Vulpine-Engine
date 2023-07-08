#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;

out vec4 _vertexColor;
out vec3 _vertexPosition;

out vec4 _fragColor;


void main()
{
    _fragColor = _vertexColor;
}