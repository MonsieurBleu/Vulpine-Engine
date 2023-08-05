#version 460

layout (location = 0) uniform ivec2 _iResolution;
layout (location = 1) uniform float _iTime;
layout (location = 2) uniform mat4 _cameraMatrix;
layout (location = 3) uniform vec3 _cameraPosition; 
layout (location = 4) uniform mat4 _modelMatrix;
layout (location = 5) uniform mat4 _modelRotation;
layout (location = 6) uniform vec3 _modelScale;
layout (location = 7) uniform vec3 _modelPosition;

in vec3 color;
in vec3 normal;
in vec3 position;

out vec4 fragColor;

void main()
{
    fragColor.a = 0.0;
    fragColor.rgb = color;

};