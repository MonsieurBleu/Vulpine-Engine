#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

layout (location = 0) in vec3 _position;
layout (location = 1) in vec4 _color;
layout (location = 2) in vec4 _uvType;

out vec2 uv;
out vec4 color;
out flat int type;
out float aspectRatio;

void main()
{
    uv = _uvType.xy;
    color = _color;
    type = int(_uvType.z);
    aspectRatio = _uvType.a;

    vec3 position = (_modelMatrix * vec4(_position, 1.0)).rgb;
    position.xy *= vec2(_iResolution.yx)/float(_iResolution.y);
    gl_Position = vec4(position, 1.0);
}

