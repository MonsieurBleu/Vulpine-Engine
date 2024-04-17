#include <Skeleton.hpp>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <string.h>
#include "Animation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

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
    {
        state[i] = state[i] * at(i).t;
    }
}

void SkeletonAnimationState::applyAnimations(float time, std::vector<std::pair<AnimationRef, float>> animations)
{
    std::vector<std::vector<keyframeData>> keyframes(animations.size(), std::vector<keyframeData>());

    for (size_t i = 0; i < animations.size(); i++)
    {
        keyframes[i] = animations[i].first->getCurrentFrames(time);
    }

    std::vector<float> factors(animations.size());
    for (size_t i = 0; i < animations.size(); i++)
    {
        factors[i] = animations[i].second;
    }

    // horrible horrible code
    for (size_t i = 0; i < size(); i++)
    {
        keyframeData data = keyframes[0][i];
        vec3 trans = data.translation;
        quat r = data.rotation;
        vec3 s = data.scale;

        for (size_t j = 1; j < animations.size(); j++)
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

void SkeletonAnimationState::applyKeyframes(std::vector<keyframeData> keyframes)
{
    for (int i = 0; i < size(); i++)
    {
        keyframeData data = keyframes[i];
        mat4 t = mat4(1);
        t = translate(t, data.translation);
        t = t * mat4_cast(data.rotation);
        t = scale(t, data.scale);

        (*this)[i] = t;
    }
}

void Skeleton::traverse(std::function<void(int, SkeletonBone &)> f)
{
    const size_t s = size();
    for (size_t i = 0; i < s; i++)
        f(i, at(i));
}

void SkeletonAnimationState::generate()
{
    glGenBuffers(1, handle.get());
}

SkeletonAnimationState::SkeletonAnimationState()
{
}

SkeletonAnimationState::SkeletonAnimationState(SkeletonRef s) : std::vector<mat4>(s->getSize()), skeleton(s)
{
    handle = std::make_shared<uint>();
    std::fill(this->begin(), this->end(), mat4(1));
    generate();
    send();
};

SkeletonAnimationState::~SkeletonAnimationState()
{
    if (handle.get() && *handle && handle.use_count() == 1)
        glDeleteBuffers(1, handle.get());
}

void SkeletonAnimationState::send()
{
    if (handle.get() && *handle)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, *handle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mat4) * size(), data(), GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void SkeletonAnimationState::update()
{
    if (handle.get() && *handle)
        glNamedBufferSubData(*handle, 0, sizeof(mat4) * size(), data());
}

void SkeletonAnimationState::activate(int location)
{
    if (handle.get() && *handle)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, *handle);
}