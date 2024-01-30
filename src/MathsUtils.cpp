#include <MathsUtils.hpp>
#include <Constants.hpp>

vec3 PhiThetaToDir(vec2 pt)
{
    vec3 v;
    v.y = sin(pt.y);
    float phiS = cos(pt.y);
    v.x = phiS*cos(pt.x);
    v.z = phiS*sin(pt.x);
    return v;
}

vec2 getPhiTheta(vec3 v)
{
    float theta = asin(v.y);
    float phiS = cos(theta);
    float phi = atan2(v.z/phiS, v.x/phiS);
    return vec2(phi, theta);
}

vec3 setPhi(vec3 v, float p)
{
    float theta = asin(v.y);
    float phiS = cos(theta);
    v.x = cos(p)*phiS;
    v.z = sin(p)*phiS;
    return v;
}

vec3 setTheta(vec3 v, float t)
{
    float phi = getPhiTheta(v).x;
    float phiS = cos(t);
    v.y = sin(t);
    v.x = cos(phi)*phiS;
    v.z = sin(phi)*phiS;
    return v;
}

vec3 hsv2rgb(vec3 hsv)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(hsv.x) + vec3(K)) * vec3(6.0) - vec3(K.w));
    return hsv.z * mix(vec3(K.x), clamp(p - vec3(K.x), vec3(0.0), vec3(1.0)), hsv.y);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(vec2(c.b, c.g), vec2(K.w, K.z)), vec4(vec2(c.g, c.b), vec2(K.x, K.y)), step(c.b, c.g));
    vec4 q = mix(vec4(vec3(p.x, p.y, p.w), c.r), vec4(c.r, vec3(p.y, p.z, p.x)), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// float getHue(vec3 c);

float getSaturation(vec3 c)
{
    float s2 = step(c.b, c.g);

    float pX = mix(c.b, c.g, s2);
    float qY = mix(c.g, c.b, s2);

    float s = step(pX, c.r);

    float qX = mix(pX, c.r, s);
    float qW = mix(c.r, pX, s);

    float d = qX - min(qW, qY);
    return d/(qX+1E-10f);
}

float getValue(vec3 c)
{
    return mix(c.b, c.g, step(c.b, c.g));
}

float gold_noise3(vec3 coordinate, float seed){
    return 0.5 - fract(tan(distance(coordinate*vec3(seed+PHI*00000.1), vec3(PHI*00000.1, PI*00000.1, E)))*SQR2*10000.0);
}