#define DIFFUSE
#define SPECULAR
#define RIM
float diffuseIntensity = 0.5;
float specularIntensity = 0.5;
float fresnelIntensity = 0.25;
vec3 ambientLight = vec3(0.5);

vec3 getDSF(vec3 lightDirection, vec3 lightColor)
{
    vec3 diffuseColor = lightColor;
    vec3 specularColor = lightColor;
    vec3 fresnelColor = lightColor;

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
    
    // rim *= pow(nDotL, 1.0) - 0.5;
#endif
    vec3 rimResult = rim*fresnelIntensity*fresnelColor;

    return diffuseResult + specularResult + rimResult;
}

vec3 getMultiLightDFS()
{
    int id = 0;
    vec3 result = vec3(0.0);
    while(true)
    {
        Light light = lights[id];
        vec3 lightResult = vec3(0.0);
        switch(light.stencil.a)
        {
            case 0 :
                return (result + ambientLight)* color;
                break;

            case 1 :
                lightResult = getDSF(light.direction.xyz, light.color.rgb)*light.color.a;
                break;

            case 2 : 
            {
                float maxDist = max(light.direction.x*100.0, 0.0001);
                float distFactor = max(maxDist - distance(position, light.position.xyz), 0.f)/maxDist;
                distFactor = pow(distFactor, 2.0);
                vec3 direction = -normalize( position - light.position.xyz);
                // vec3 direction = vec3(1.0);

                lightResult = getDSF(direction, light.color.rgb)*distFactor*light.color.a;
            }
                break;

            default : break;
        }
        
        result += lightResult;

        id ++;
    }

    return (result + ambientLight)* color;
}