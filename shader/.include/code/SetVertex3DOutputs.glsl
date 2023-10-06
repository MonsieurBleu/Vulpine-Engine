#ifndef USING_VERTEX_TEXTURE_UV
    color = _color;
#else
    uv = vec2(_uv.x , 1.0 - _uv.y);
#endif
    normal = ((_modelMatrix * vec4(_normal, 1.0)).rgb)/_modelScale - _modelPosition;
    viewNormal = normalize(normal);
    position = (_modelMatrix * vec4(_positionInModel, 1.0)).rgb;
