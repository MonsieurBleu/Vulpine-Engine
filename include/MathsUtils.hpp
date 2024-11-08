#pragma once

#include <glm/glm.hpp>

using namespace glm;



float random01Vec2(vec2 uv);

vec3 viewToWorld(vec4 pos, const mat4& m);

vec3 screenPosToModel(vec2 screenPos);

/****** ROTATION & DIRECTION UTILS******/
    vec3 slerpDirClamp(vec3 dir1, vec3 dir2, float t, vec3 wfront = vec3(0, 0, 1));

    vec3 PhiThetaToDir(vec2 pt);

    vec2 getPhiTheta(vec3 v);

    vec3 setPhi(vec3 v, float p);

    vec3 setTheta(vec3 v, float t);

    vec3 rotateVec(vec3 front, vec3 axis, float angle);

    vec2 toHvec2(const vec3 &v);

    float angle(const vec2 &v1, const vec2& v2);

    quat directionToQuat(vec3 dir);
    
    vec3 directionToEuler(vec3 dir);


/****** COLOR UTILS******/
    vec3 hsv2rgb(vec3 hsv);

    vec3 rgb2hsv(vec3 c);

    // float getHue(vec3 c);

    float getSaturation(vec3 c);

    float getValue(vec3 c);

    float gold_noise3(vec3 coordinate, float seed);

    vec3 ColorHexToV(uint hex);
