    color = _color;
    normal = (_modelRotation * vec4(_normal, 1.0)).rgb;
    viewNormal = normalize(normal);
    position = (_modelMatrix * vec4(_positionInModel, 1.0)).rgb;
