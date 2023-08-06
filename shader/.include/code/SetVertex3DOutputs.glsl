    color = _color;
    normal = (_modelRotation * vec4(_normal, 1.0)).rgb;
    mat3 normalMatrix = transpose(inverse(mat3(_cameraViewMatrix * _modelMatrix)));
    viewNormal = normalize(normalMatrix * _normal);
    position = (_modelMatrix * vec4(_positionInModel, 1.0)).rgb;