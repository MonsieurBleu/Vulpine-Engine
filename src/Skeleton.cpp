#include <Skeleton.hpp>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <string.h>
#include "Animation.hpp"

const SkeletonBone &Skeleton::operator[](int i)
{
    return at(i);
}

#include <iostream>

void Skeleton::applyGraph(SkeletonAnimationState &state)
{
    const size_t s = size();

    if (state.size() != s)
        return;

    for (size_t i = 0; i < s; i++)
    {
        SkeletonBone &b = at(i);
        if (b.parent >= 0)
            state[i] = state[b.parent] * state[i];
    }

    for (size_t i = 0; i < s; i++)
        state[i] = state[i] * at(i).t;
}

void SkeletonAnimationState::applyAnimations(float time, std::vector<std::pair<AnimationRef, float>> animations)
{
    std::vector<std::vector<keyframeData>> keyframes(animations.size(), std::vector<keyframeData>());

    for (int i = 0; i < animations.size(); i++)
    {
        keyframes[i] = animations[i].first->getCurrentFrames(time);
    }

    std::vector<float> factors(animations.size());
    for (int i = 0; i < animations.size(); i++)
    {
        factors[i] = animations[i].second;
    }

    // horrible horrible code
    for (int i = 0; i < size(); i++)
    {
        keyframeData data = keyframes[0][i];
        vec3 trans = data.translation;
        quat r = data.rotation;
        vec3 s = data.scale;
        for (int j = 1; j < animations.size(); j++)
        {
            if (keyframes[j].size() == 0)
                continue;

            keyframeData data = keyframes[j][i];
            float factor = factors[j];

            trans = mix(trans, data.translation, factor);
            r = lerp(r, data.rotation, factor);
            s = mix(s, data.scale, factor);
        }

        mat4 t = mat4(1);
        t = translate(t, trans);
        t = t * mat4_cast(r);
        t = scale(t, s);

        (*this)[i] = t;
    }
}

void Skeleton::traverse(std::function<void(int, SkeletonBone &)> f)
{
    const size_t s = size();
    for (size_t i = 0; i < s; i++)
        f(i, at(i));
}

SkeletonAnimationState::SkeletonAnimationState()
{
    glGenBuffers(1, &handle);
}

SkeletonAnimationState::~SkeletonAnimationState()
{
    if (handle)
        glDeleteBuffers(1, &handle);
}

void SkeletonAnimationState::send()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mat4) * size(), data(), GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SkeletonAnimationState::update()
{
    glNamedBufferSubData(handle, 0, sizeof(mat4) * size(), data());
}

void SkeletonAnimationState::activate(int location)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
}