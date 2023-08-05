#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 9) uniform mat4 _cameraProjectionMatrix; 

layout (location = 0) in vec2 vertexPosition;

out vec2 uvScreen;


out vec2 ViewRay;
float camFOV = 50.0;
float aspectRatio = float(iResolution.x)/float( iResolution.y);

void main()
{
    uvScreen = vertexPosition*0.5 + 0.5;

    float TanHalfFOV = tan(radians(camFOV / 2.0f));
    ViewRay = vertexPosition * vec2(aspectRatio, 1.0) * TanHalfFOV;
    // ViewRay = vec2(0.0);

    gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0.99999, 1.0);
}