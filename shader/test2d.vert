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

    screenPosition *= vec2(1080.0, 1080.0);
    
    screenPosition *= quadScale;
    screenPosition.x = screenPosition.x*cos(quadRotationDepth.x) - screenPosition.y*sin(quadRotationDepth.x);
    screenPosition.y = screenPosition.x*sin(quadRotationDepth.x) + screenPosition.y*cos(quadRotationDepth.x);
    
    screenPosition /= vec2(1920.0, 1080.0);

    screenPosition += quadPosition;


    vertexColor = vec4(1.0);
    vertexColor.rgb = vec3(abs(quadRotationDepth.x)*5.0);

    // vertexColor.r = distance(vertexPosition, vec2(0.0));
    
    /*
        TODO : fix rotation
    */


    
    gl_Position = vec4(screenPosition.x, screenPosition.y, quadRotationDepth.y, 1.0);
};