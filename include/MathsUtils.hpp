#pragma once

#include <glm/glm.hpp>

using namespace glm;

#include <vector>



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


/***** GEOMETRICAL UTILS ******/
vec3 projectPointOntoPlane(vec3 point, vec3 planePoint, vec3 planeNormal);

vec3 rayAlignedPlaneIntersect(vec3 origin, vec3 direction, float axis, float axisPosition);


/****** COLOR UTILS******/
    vec3 hsv2rgb(vec3 hsv);

    vec3 rgb2hsv(vec3 c);

    // float getHue(vec3 c);

    float getSaturation(vec3 c);

    float getValue(vec3 c);

    float gold_noise3(vec3 coordinate, float seed);

    vec3 ColorHexToV(uint hex);

/****** SPLINE ******/

/**
 * @brief This function computes the B-Spline curve given a set of control points
 * 
 * @param points - the control points
 * @param spline - the output spline (as a list of points)
 * @param numSegments - the number of segments to use for the spline (default is 10)
 */
void BSpline(const std::vector<vec3> &points, std::vector<vec3> &spline, int numSegments = 10);

/**
 * @brief This function computes the bezier curve given 4 control points
 * @param p0 - the first control point (the start point)
 * @param p1 - the second control point
 * @param p2 - the third control point
 * @param p3 - the fourth control point (the end point)
 * @param spline - the output spline (as a list of points)
 * @param numSegments - the number of segments to use for the spline (default is 10)
 */
void BezierCurve(const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p3, std::vector<vec3> &spline, int numSegments = 10);