struct Light
{
    vec4 position;     
    vec4 color;         
    vec4 direction;       
    ivec4 stencil;  
};

// 0, 0, 0
// 2, 3, 4
// 6, 7, 0

/// 0, 0, 0, 1
/// 2, 3, 4, 5
/// 6, 7, 0, 0

layout (location = 10) uniform uint lightCounter;

layout (std430, binding = 0) readonly buffer lightsBuffer 
{
    Light lights[];
};
