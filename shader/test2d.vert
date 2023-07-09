#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 3) uniform vec3 CameraPosition;
layout (location = 4) uniform vec3 CameraDirection;


layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 quadPosition;
layout (location = 2) in vec2 quadScale;
layout (location = 3) in vec2 quadRotationDepth;

out vec4 vertexColor;
out vec2 quadUv;


void main()
{
    vec2 screenPosition = vertexPosition;
    screenPosition.x *= 1080.0/1920.0;
    screenPosition += quadPosition;
    screenPosition *= 0.25;

    vertexColor = vec4(1.0);
    vertexColor.rg = (vertexPosition + 1.0)*0.5;
    
    
    gl_Position = vec4(screenPosition.x, screenPosition.y, 0.0, 1.0);

    // gl_Position = MVP * vec4(_vertexPosition, 1.0);
};