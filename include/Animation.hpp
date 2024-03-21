#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <cstring>
#include "Skeleton.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

typedef unsigned char byte;
enum AnimationType : byte
{
    ANIMATION_TYPE_MATRIX,
    ANIMATION_TYPE_QUATERNION,
};

struct AnimationFileHeader
{
    int magicNumber; // should be "VANI" in ASCII or 0x56414e49 in hex
    AnimationType type;
    char animationName[128];
    float duration;
    unsigned int totalBoneNumber;
    unsigned int animatedBoneNumber;

    AnimationFileHeader(AnimationType type, const char *name, float duration, unsigned int totalBoneNumber, unsigned int animatedBoneNumber)
        : type(type), duration(duration), totalBoneNumber(totalBoneNumber), animatedBoneNumber(animatedBoneNumber)
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

struct AnimationKeyframeDataMatrix
{
    float time;
    mat4 matrix;
};

struct AnimationKeyframeDataQuaternion
{
    float time;
    quat quaternion;
};

class Animation;

typedef std::shared_ptr<Animation> AnimationRef;

class Animation
{
private:
    const AnimationType animType;
    std::string name;
    float length;

    std::vector<std::vector<AnimationKeyframeDataMatrix>> keyframes;

    SkeletonRef skeleton;

public:
    Animation(
        AnimationType type, 
        const std::string &name, 
        float length, 
        std::vector<std::vector<AnimationKeyframeDataMatrix>> keyframes,
        SkeletonRef skeleton)
        : animType(type), name(name), length(length), keyframes(keyframes), skeleton(skeleton) {}

    static AnimationRef load(SkeletonRef skeleton, const std::string &filename);

    AnimationType getType() const { return animType; }
    const std::string getName() const { return name; }
    float getLength() const { return length; }
    const std::vector<std::vector<AnimationKeyframeDataMatrix>> &getKeyframes() { return keyframes; }
    const std::vector<AnimationKeyframeDataMatrix> &getKeyframe(int i) { return keyframes[i]; }
    const std::vector<AnimationKeyframeDataMatrix> &operator[](int i) { return keyframes[i]; }
    int getKeyframeNumber() const { return keyframes.size(); }
    void apply(float time, SkeletonAnimationState &state) const;
};