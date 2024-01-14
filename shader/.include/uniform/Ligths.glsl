struct Light
{
    vec4 position;     
    vec4 color;         
    vec4 direction;       
    ivec4 stencil;  
    mat4 _rShadowMatrix;
};

layout (location = 15) uniform vec3 ambientLight;

layout (binding = 16) uniform sampler2D bShadowMaps[16];

layout (std430, binding = 0) readonly buffer lightsBuffer 
{
    Light lights[];
};
