#pragma once 

#include <vector>
#include <string>

#include <functional>

#include <glm/glm.hpp>
using namespace glm;

struct SkeletonBone
{
    mat4 t = mat4(1);
    int parent = 0;
    int children[15] = {0};
};

class SkeletonAnimationState : public std::vector<mat4>
{};


class Skeleton : protected std::vector<SkeletonBone>
{
    private :
     
    public : 
        const SkeletonBone& operator[](int i);
        void load(const std::string &filename);
        int getSize(){return size();};

        void applyGraph(SkeletonAnimationState &state);

        void traverse(std::function<void(int, SkeletonBone &)> f);
};


