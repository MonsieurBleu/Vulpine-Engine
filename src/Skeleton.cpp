#include <Skeleton.hpp>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <string.h>

const SkeletonBone& Skeleton::operator[](int i)
{
    return at(i);
}

#include <iostream>

void Skeleton::applyGraph(SkeletonAnimationState &state)
{
    const size_t s = size();

    if(state.size() != s) return;

    for(size_t i = 0; i < s; i++)
    {
        SkeletonBone &b = at(i);
        if(b.parent >= 0) state[i] = state[b.parent] * state[i];
    }

    for(size_t i = 0; i < s; i++)
        state[i] = state[i] * at(i).t;
}

void Skeleton::traverse(std::function<void(int, SkeletonBone &)> f)
{
    const size_t s = size();
    for(size_t i = 0; i < s; i++) f(i, at(i));
}

SkeletonAnimationState::SkeletonAnimationState()
{
    glGenBuffers(1, &handle);
}

SkeletonAnimationState::~SkeletonAnimationState()
{
    if(handle) glDeleteBuffers(1, &handle);
}

void SkeletonAnimationState::send()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mat4)*size(), data(), GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SkeletonAnimationState::update()
{
    glNamedBufferSubData(handle, 0, sizeof(mat4)*size(), data());
}

void SkeletonAnimationState::activate(int location)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, handle);
}