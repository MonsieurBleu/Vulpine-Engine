layout (location = 0) uniform ivec2 _iResolution;
layout (location = 1) uniform float _iTime;
layout (location = 2) uniform mat4 _cameraMatrix;
layout (location = 3) uniform mat4 _cameraViewMatrix; 
layout (location = 4) uniform mat4 _cameraProjectionMatrix; 
layout (location = 5) uniform vec3 _cameraPosition; 
layout (location = 6) uniform vec3 _cameraDirection; 
/*
    TODO : add this uniform
*/
layout (location = 7) uniform mat4 _cameraInverseViewMatrix;