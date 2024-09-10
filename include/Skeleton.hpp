#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>

#include <glm/glm.hpp>
using namespace glm;

#include "Animation.hpp"

struct SkeletonBone
{
    mat4 t = mat4(1);
    int parent = 0;
    int children[15] = {0};
};

class SkeletonAnimationState;

class Skeleton : public std::vector<SkeletonBone>
{
private:

public:

    std::string name;

    const SkeletonBone &operator[](int i);
    void load(const char *filename);
    int getSize() { return size(); };

    void applyGraph(SkeletonAnimationState &state);

    /*
        Stackless cache optimized layered graph traversal

        This method traverse the graph layer by layer.
        This can be used to apply graph hierarcy to any
        element.

        This method is very barebones due to the specific
        arangement of the vulpineSkeleton bones in memory.
        It can be replaced by a simple for loop on all
        elements.
    */
    void traverse(std::function<void(int, SkeletonBone &)> f);
};

typedef std::shared_ptr<Skeleton> SkeletonRef;

class Animation;

typedef std::shared_ptr<Animation> AnimationRef;

class SkeletonAnimationState : public std::vector<mat4>
{
    private:
        // uint handle = 0;
        std::shared_ptr<uint> handle;

    public:
        std::string name;

        SkeletonAnimationState(SkeletonRef s);
        SkeletonAnimationState();
        ~SkeletonAnimationState();

        SkeletonRef skeleton;

        /**
         * @brief TODO : delete... applies a number of animations to the state
         *
         * @param time the time
         * @param animations a vector that contains animations as well as their "factor" (how much it affects the final animation, ideally should sum up to 1 but we ball)
         */
        void applyAnimations(float time, std::vector<std::pair<AnimationRef, float>> animations);

        void applyKeyframes(std::vector<keyframeData> keyframes);

        void generate();
        void send();
        void update();
        void activate(int location);
};
