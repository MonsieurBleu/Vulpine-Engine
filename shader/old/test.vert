#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;
layout (location = 3) uniform vec3 CameraPosition;
layout (location = 4) uniform vec3 CameraDirection;


layout (location = 0) in vec3 Vpos;
layout (location = 1) in vec3 offsets;

out vec4 vertex_color;
out vec3 vertex_position;


void main()
{
    // gl_Position = MVP * vec4(Vpos+offsets[gl_InstanceID], 1.0);

    // vec3 finalpos = Vpos + offsets;

    vec3 finalpos = Vpos;
    finalpos.x += float(gl_InstanceID%1000) * 10.0;
    finalpos.z += float(gl_InstanceID/1000) * 10.0;


    // float offset = cos(finalpos.x * 0.1)*150.0;
    float offset = -distance(finalpos, vec3(sin(iTime)*50.0 + 180.0, 0.0, cos(iTime)*50.0 + 180.0)) + 170.0;

    if(offset < 0) offset = 0.0;

    finalpos.y += offset/3.0;

    finalpos.y += cos(finalpos.x*0.005*iTime)*5.0;


    gl_Position = MVP * vec4(finalpos +  vec3(500.0, 0.0, 0.0), 1.0);

    // vertex_color.rgb = Vpos + 0.5;
    // vertex_color.rgb = offsets/10.0 + 0.5;

    // vertex_color.gb = abs(finalpos.xz-Vpos.xz - CameraPosition.xz)/1000.0 + 0.5;
    // vertex_color.rgb = 1.0 - vec3(length(finalpos-CameraPosition))/256.0;
    vertex_color.a = 1.0;

    vertex_color.rgb = vec3(0.85);

    vertex_position = finalpos-Vpos;

    vertex_color.rgb = vec3(0.0);

};