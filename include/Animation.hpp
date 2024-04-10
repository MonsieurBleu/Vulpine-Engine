#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <cstring>
#include <unordered_map>
#include "Skeleton.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

typedef unsigned char byte;

struct AnimationFileHeader
{
    int magicNumber; // should be "VANI" in ASCII or 0x56414e49 in hex
    char animationName[128];
    float duration;
    unsigned int totalBoneNumber;
    unsigned int animatedBoneNumber;

    AnimationFileHeader(const char *name, float duration, unsigned int totalBoneNumber, unsigned int animatedBoneNumber)
        : duration(duration), totalBoneNumber(totalBoneNumber), animatedBoneNumber(animatedBoneNumber)
    {
        strncpy(animationName, name, 127);
    }

    AnimationFileHeader() {}
};

enum AnimationBehaviour : byte
{
    /** The value from the default node transformation is taken*/
    AnimationBehaviour_DEFAULT = 0x0,

    /** The nearest key value is used without interpolation */
    AnimationBehaviour_CONSTANT = 0x1,

    /** The value of the nearest two keys is linearly
     *  extrapolated for the current time value.*/
    AnimationBehaviour_LINEAR = 0x2,

    /** The animation is repeated.
     *
     *  If the animation key go from n to m and the current
     *  time is t, use the value at (t-n) % (|m-n|).*/
    AnimationBehaviour_REPEAT = 0x3,
};

struct AnimationBoneData
{
    unsigned int boneID;
    AnimationBehaviour preStateBehaviour;
    AnimationBehaviour postStateBehaviour;
    unsigned int keyframeNumber;
};

struct AnimationKeyframeData
{
    float time;
    vec3 translation;
    quat rotation;
    vec3 scale;
};

// similar to animationkeyframedata but has no time component
struct keyframeData
{
    vec3 translation;
    quat rotation;
    vec3 scale;
};

class Animation;

typedef std::shared_ptr<Animation> AnimationRef;

class Animation
{
private:
    std::string name;
    float length;

    std::vector<std::vector<AnimationKeyframeData>> keyframes;
    std::vector<int> currentKeyframeIndex;

    SkeletonRef skeleton;

public:
    Animation(){};

    Animation(
        const std::string &name,
        float length,
        std::vector<std::vector<AnimationKeyframeData>> &keyframes,
        SkeletonRef skeleton)
        : name(name), length(length), keyframes(keyframes), skeleton(skeleton)
    {
        currentKeyframeIndex.resize(keyframes.size(), 0);
    }

    static AnimationRef load(SkeletonRef skeleton, const std::string &filename);

    const std::string getName() const { return name; }
    float getLength() const { return length; }
    const std::vector<std::vector<AnimationKeyframeData>> &getKeyframes() { return keyframes; }
    const std::vector<AnimationKeyframeData> &getKeyframe(int i) { return keyframes[i]; }
    const std::vector<AnimationKeyframeData> &operator[](int i) { return keyframes[i]; }
    int getKeyframeNumber() const { return keyframes.size(); }
    std::vector<keyframeData> getCurrentFrames(float time);

    friend class Skeleton;
};