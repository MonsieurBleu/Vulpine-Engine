#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;

in vec4 vertex_color;
in vec3 vertex_position;

out vec4 frag_color;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float random (vec2 st) {
    return fract(sin(dot(st.xy,vec2(12.9898,78.233)))*43758.5453123);
}

void main()
{
    frag_color.rgb = vertex_position/256.0;

    // frag_color.rgb = vec3(0.75);
    // frag_color.a = 1.0;
    // frag_color.r = vertex_position.y*0.005;

    // frag_color.rgb = hsv2rgb(frag_color.rgb);
    
    // frag_color.bg = cos(vertex_position.xz / 50.0)*0.5 + 0.5;
    // frag_color.bg = frag_color.rb*0.65 + 0.25;
    // frag_color.r = 0.1;

    frag_color.a = 1.0;
}