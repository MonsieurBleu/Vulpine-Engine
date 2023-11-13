#version 460

layout (location = 0) uniform ivec2 iResolution;
layout (location = 1) uniform float iTime;
layout (location = 2) uniform mat4 MVP;

in vec2 uvScreen;
out vec3 fragColor;

// https://www.shadertoy.com/view/7scSDj
// https://en.wikipedia.org/wiki/Gall%E2%80%93Peters_projection
// https://stackoverflow.com/questions/10473852/convert-latitude-and-longitude-to-point-in-3d-space

float rand(vec2 co)
{
    return fract(sin(dot(co.xy,vec2(12.9898,78.233)))*43758.5453123);
}

// https://www.shadertoy.com/view/wtsSW4
#define PHI 1.61803398874989484820459 // Golden Ratio   
#define PI  3.14159265358979323846264 // PI
#define SQ2 1.41421356237309504880169 // Square Root of Two
#define E   2.71828182846
float gold_noise3(in vec3 coordinate, in float seed){
    return fract(tan(distance(coordinate*(seed+PHI*00000.1), vec3(PHI*00000.1, PI*00000.1, E)))*SQ2*10000.0);
}

// https://www.shadertoy.com/view/MlSfzz
#define nsin(x) (sin(x) * 0.5 + 0.5)
void draw_auroras(inout vec3 color, vec2 uv) {
    const vec3 aurora_color_a = vec3(0.0, 1.2, 0.8);
    const vec3 aurora_color_b = vec3(0.0, 0.5, 0.9);
    
    float t = nsin(-iTime + uv.x * 100.0) * 0.075 + nsin(iTime + uv.x * distance(uv.x, 0.5) * 100.0) * 0.1 - 0.5;
    t = 1.0 - smoothstep(uv.y - 4.0, uv.y * 2.0, t);
    
    vec3 final_color = mix(aurora_color_a, aurora_color_b, clamp(1.0 - uv.y * t, 0.0, 1.0));
    final_color += final_color * final_color;
    final_color *= 1.2;
    color += final_color * t * (t + 0.5) * 0.75;
}

void draw_stars(inout vec3 color, vec3 uv) {
    float t = sin(iTime * 2.0 * gold_noise3(-uv, 1)) * 0.5 + 0.5;
    //color += step(0.99, stars) * t;
    color += smoothstep(0.975, 1.0, gold_noise3(uv, 10)) * t;
}


void main()
{
    vec2 uv = uvScreen;

    // uvSky.x = 0.5 + atan(reflectDir.z, -reflectDir.x) / (2.0*PI);
    // uvSky.y = -reflectDir.y*0.5 + 0.5;

    // vec3 dir;
    // dir.y = -(uv.y - 0.5)*2.0;
    // float tmp = (uv.x - 0.5)*2.0*PI;
    // tmp = tan(tmp);
    // dir.x = acos(tmp);
    // dir.z = asin(tmp);

    // dir.x *= sign(uv.x);


    vec3 position;
    float lonAngle = PI*(0.5-uvScreen.y);
    float latAngle = 2.0*PI*(0.5-uvScreen.x);
    position.y = sin(lonAngle);
    float latRadius = cos(lonAngle);
    position.x = latRadius*cos(latAngle);
    position.z = latRadius*sin(latAngle);

    // position.z /= position.x;
    position.z = position.z;

    uv = position.zy;



    float iSpeed = 50.0;
    float iDensity = 1.0;
    float iStarSize = 0.001;
    
    /*
    // 2D EXAMPLE FROM SHADERTOY 
    vec2 filteredRes = vec2(uv.x - mod(uv.x, iStarSize), uv.y - mod(uv.y, iStarSize));
    vec2 st = filteredRes;
    st *= 10.0;

    vec2 ipos = floor(st);
    vec2 fpos = fract(st);

    float isStar = (pow(rand(fpos), 100.0 / iDensity)) * sin((iTime/(100.0/iSpeed)) * (rand(filteredRes) * (3.14159)));
    isStar *= 5.0;
    float nebula = (rand(fpos) ) * 0.2;
    fragColor = vec3(max(isStar, (sin(nebula) + 1.0)/8.0), isStar, max(isStar, (cos(nebula) + 1.0)/8.0));
    */

    // 3D Version 
    // vec3 filteredRes = position - mod(position, vec3(iStarSize));
    // vec3 fpos = fract(filteredRes*10.0);
    // float seed = 10;

    // float isStar = (pow(gold_noise3(fpos, seed), 100.0 / iDensity)) * sin((iTime/(100.0/iSpeed)) * (gold_noise3(filteredRes, seed) * PI));
    // isStar *= 2.0;
    // /// nebula is useless
    // float nebula = gold_noise3(fpos, seed)*0.2;
    // fragColor = vec3(max(isStar, (sin(nebula) + 1.0)/8.0), isStar, max(isStar, (cos(nebula) + 1.0)/8.0));


    // fragColor = vec3(uv, 0.0);
    // fragColor = position*0.5 + 0.5;
    // fragColor = position.xyz;
    // fragColor = vec3(gold_noise3(fpos, seed));
    // fragColor.r = 0;

    // fragColor = vec3(sin(nebula*PI*5.0), cos(nebula*PI*5.0), 0.0);

    draw_stars(fragColor, position);
    // draw_auroras(fragColor, vec2(0.5*abs(latAngle), 0.5 + 0.5*uv.x) + 0.05);
    
    vec2 uvAuroras = vec2(0.5*abs(latAngle), 0.5 + 0.5*uv.x) + 0.05;
    // vec2 uvAuroras = vec2(latAngle, lonAngle);
    draw_auroras(fragColor, uvAuroras);

    float moon = 
        smoothstep(
        0.05,
        0.1,
        clamp(
            10.0*pow(
                distance(position, normalize(vec3(1.0, 1.0, 0.0))), 
                2.0), 
            0, 
            1)
    );

    vec3 moonColor = mix(
        vec3(1.0, 0.95, 0.85),
        vec3(1.0, 0.9, 0.85)*0.75,
        rand(vec2(latAngle, lonAngle))
    );

    fragColor = mix(
        moonColor, 
        fragColor, 
        moon
    );
}
