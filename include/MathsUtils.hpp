#ifndef MATHS_UTILS_HPP
#define MATHS_UTILS_HPP

#include <glm/glm.hpp>

using namespace glm;

vec2 getPhiTheta(vec3 v);

vec3 setPhi(vec3 v, float p);

vec3 setTheta(vec3 v, float t);

vec3 hsv2rgb(vec3 hsv);

vec3 rgb2hsv(vec3 c);

// float getHue(vec3 c);

float getSaturation(vec3 c);

float getValue(vec3 c);


#endif