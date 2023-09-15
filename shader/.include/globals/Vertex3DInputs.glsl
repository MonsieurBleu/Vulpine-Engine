layout (location = 0) in vec3 _positionInModel;
layout (location = 1) in vec3 _normal;

#ifndef USING_VERTEX_TEXTURE_UV
    layout (location = 2) in vec3 _color;
#else
    layout (location = 2) in vec2 _uv;
#endif
