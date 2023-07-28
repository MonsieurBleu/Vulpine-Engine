#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;
layout (location = 1) in vec3 Normal;

out vec3 vertexColor;
out vec3 vertexNormal;


void main()
{
    vertexColor = Color;
    vertexNormal = Normal;

    gl_Position = MVP * vec4(Position, 1.0);
};