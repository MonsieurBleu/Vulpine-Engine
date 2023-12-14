#include <MathsUtils.hpp>

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
