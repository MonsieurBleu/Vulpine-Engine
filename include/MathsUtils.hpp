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
inline void BSpline(const std::vector<vec3> &points, std::vector<vec3> &spline, int numSegments = 10)
{
    if (points.size() < 2)
        return;

    std::vector<vec3> controlPoints = points;
    controlPoints.insert(controlPoints.begin(), controlPoints[0]);
    controlPoints.push_back(controlPoints.back());

    for (int i = 0; i < controlPoints.size() - 3; i++)
    {
        for (int j = 0; j < numSegments; j++)
        {
            float t = (float)j / (numSegments - 1);
            float t2 = t * t;
            float t3 = t2 * t;

            vec3 p = 0.5f * ((-t3 + 2 * t2 - t) * controlPoints[i] +
                             (3 * t3 - 5 * t2 + 2) * controlPoints[i + 1] +
                             (-3 * t3 + 4 * t2 + t) * controlPoints[i + 2] +
                             (t3 - t2) * controlPoints[i + 3]);

            spline.push_back(p);
        }
    }
}