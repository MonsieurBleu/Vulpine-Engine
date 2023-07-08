#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 3) uniform vec3 CameraPosition;
layout (location = 4) uniform vec3 CameraDirection;


layout (location = 0) in vec3 vertexPosition;

out vec4 _vertexColor;
out vec3 _vertexPosition;


void main()
{
    _vertexPosition = vertexPosition;
    _vertexColor = vec4(1.0);

    gl_Position = MVP * vec4(_vertexPosition, 1.0);
};