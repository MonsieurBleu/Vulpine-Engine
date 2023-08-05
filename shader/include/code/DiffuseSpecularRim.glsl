    /*
        UTILS
    */
    vec3 viewDir = normalize(_cameraPosition - position);
    vec3 reflectDir = reflect(-lightDirection, normal); 
    float nDotL = max(dot(lightDirection, normal), 0);

    /*
        DIFFUSE 
    */
    float diffuse = 0.0;
#ifdef DIFFUSE
    diffuse = nDotL;
#endif
    vec3 diffuseResult = diffuse*diffuseIntensity*diffuseColor;

    /*
        SPECULAR
    */
    float specular = 0.0;
#ifdef SPECULAR
    int specularExponent = 8;
    specular = pow(max(dot(reflectDir, viewDir), 0.0), specularExponent);
#endif
    vec3 specularResult = specular*specularIntensity*specularColor;

    /*
        RIM
    */
    float rim = 0.0;
#ifdef RIM 
    rim = 1.0 - dot(viewDir, normal);
    // rim *= pow(nDotL, 1.0);
    
    rim *= pow(nDotL, 1.0) - 0.5;
#endif
    vec3 rimResult = rim*rimIntensity*rimColor;

    vec3 DiffuseSpecularRimResult = color * (ambientLight + diffuseResult + specularResult + rimResult);

    // DiffuseSpecularRimResult = rimResult;
    // if(rim < 0.0) DiffuseSpecularRimResult = vec3(-rim, 0.0, 0.0);
