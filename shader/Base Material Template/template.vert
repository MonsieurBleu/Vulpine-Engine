#version 460

layout (location = 0) uniform ivec2 _iResolution;
layout (location = 1) uniform float _iTime;
layout (location = 2) uniform mat4 _cameraMatrix;
layout (location = 3) uniform vec3 _cameraPosition; 
layout (location = 4) uniform mat4 _modelMatrix;
layout (location = 5) uniform mat4 _modelRotation;
layout (location = 6) uniform vec3 _modelScale;
layout (location = 7) uniform vec3 _modelPosition;

layout (location = 0) in vec3 _position;
layout (location = 1) in vec3 _normal;
layout (location = 2) in vec3 _color;

out vec3 color;
out vec3 normal;
out vec3 position;

void main()
{
    color = _color;
    normal = (_modelRotation * vec4(_normal, 1.0)).rgb;
    position = (_modelRotation * vec4(_position*_modelScale, 1.0)).rgb + _modelPosition;
    
    gl_Position = _cameraMatrix * vec4(position, 1.0);
};