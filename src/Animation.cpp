#include "Animation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Utils.hpp"

#include <iostream>
#include <fstream>

AnimationRef Animation::load(const std::string &filename)
{
    // FILE *file = fopen(filename.c_str(), "rb");
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << TERMINAL_ERROR
                  << "Animation::load : can't open file "
                  << TERMINAL_FILENAME
                  << filename
                  << TERMINAL_ERROR
                  << ".\n"
                  << TERMINAL_RESET;
        return nullptr;
    }

    AnimationFileHeader header;
    // fread(&header, sizeof(AnimationFileHeader), 1, file);
    file.read((char *)&header, sizeof(AnimationFileHeader));

    if (header.magicNumber != 0x494e4156)
    {
        std::cerr << TERMINAL_ERROR
                  << "Animation::load => file "
                  << TERMINAL_FILENAME
                  << filename
                  << TERMINAL_ERROR
                  << " is not a valid animation file.\n"
                  << TERMINAL_RESET;
        return nullptr;
    }

    AnimationRef anim = std::make_shared<Animation>();
    // std::vector<std::vector<AnimationKeyframeData>> keyframes(skeleton->getSize(), std::vector<AnimationKeyframeData>());
    std::vector<std::vector<AnimationKeyframeData>> keyframes(header.totalBoneNumber, std::vector<AnimationKeyframeData>());

    for (uint i = 0; i < header.animatedBoneNumber; i++)
    {
        AnimationBoneData boneData;
        // fread(&boneData, sizeof(AnimationBoneData), 1, file);
        file.read((char *)&boneData, sizeof(AnimationBoneData));

        keyframes[boneData.boneID].resize(boneData.keyframeNumber);

        // fread(keyframes[boneData.boneID].data(), sizeof(AnimationKeyframeData), boneData.keyframeNumber, file);
        file.read((char *)keyframes[boneData.boneID].data(), sizeof(AnimationKeyframeData) * boneData.keyframeNumber);
    }

    // fclose(file);
    file.close();

    std::string name = header.animationName;
    // AnimationRef anim = std::make_shared<Animation>(name, header.duration, keyframes, skeleton);
    // anim->currentKeyframeIndex.resize(keyframes.size(), 0);
    anim->length = header.duration;
    anim->name = name;
    anim->keyframes = keyframes;
    return anim;
}

void Animation::getCurrentFrames(float time, float & lastTime, std::vector<int16> & currentKeyframeIndex, std::vector<keyframeData> &data)
{
    // static float lastTime = 0;

    // std::vector<keyframeData> data(keyframes.size());
    if(data.empty()) data.resize(keyframes.size());

    if(!repeat && time >= length)
    {
        // time = length;
        // lastTime = time;

        for (uint i = 0; i < keyframes.size(); i++)
        {
            if (keyframes[i].size() == 0)
            {
                data[i] = {vec3(0), quat(1, 0, 0, 0), vec3(1)};
                continue;
            }

            data[i] = {
                keyframes[i].back().translation,
                keyframes[i].back().rotation,
                keyframes[i].back().scale
            };
        }

        return;
    }

    time = fmod(time, length);

    if (time < lastTime)
    {
        std::fill(currentKeyframeIndex.begin(), currentKeyframeIndex.end(), 0);
    }

    lastTime = time;

    for (uint i = 0; i < keyframes.size(); i++)
    {
        if (keyframes[i].size() == 0)
        {
            data[i] = {vec3(0), quat(1, 0, 0, 0), vec3(1)};
            continue;
        }

        if (keyframes[i].size() == 1)
        {
            data[i].rotation = keyframes[i][0].rotation;
            data[i].translation = keyframes[i][0].translation;
            data[i].scale = keyframes[i][0].scale;
            // data[i].time = keyframes[i][0].time;
            continue;
        }

        uint nextKeyframeIndex = (currentKeyframeIndex[i] + 1) % keyframes[i].size();

        while (time > keyframes[i][nextKeyframeIndex].time)
        {
            currentKeyframeIndex[i] = nextKeyframeIndex;
            nextKeyframeIndex = (currentKeyframeIndex[i] + 1) % keyframes[i].size();

            /* TODO : fix, this can cause animations to be bugged at low frame rate*/
            if (currentKeyframeIndex[i] == 0)
                break;
        }

        // get the time difference between the two keyframes
        float deltaTime = keyframes[i][nextKeyframeIndex].time - keyframes[i][currentKeyframeIndex[i]].time;

        // get the time difference between the current time and the current keyframe
        float currentTime = time - keyframes[i][currentKeyframeIndex[i]].time;

        // get the interpolation factor
        float factor = currentTime / deltaTime;

        // interpolate translation
        vec3 translation = glm::mix(keyframes[i][currentKeyframeIndex[i]].translation, keyframes[i][nextKeyframeIndex].translation, factor);

        // interpolate rotation
        quat rotation = glm::slerp(keyframes[i][currentKeyframeIndex[i]].rotation, keyframes[i][nextKeyframeIndex].rotation, factor);

        // interpolate scale
        vec3 scale = glm::mix(keyframes[i][currentKeyframeIndex[i]].scale, keyframes[i][nextKeyframeIndex].scale, factor);

        data[i] = {translation, rotation, scale};
    }

    return;
}

void interpolateKeyframes(
    AnimationRef animA, 
    AnimationRef animB, 
    float t1, 
    float t2,
    float &lt1, 
    float &lt2,
    float a, 
    std::vector<int16> &currentKeyframeIndexA,
    std::vector<int16> &currentKeyframeIndexB,
    std::vector<keyframeData> &data)
{
    if (animA->keyframes.size() != animB->keyframes.size())
    {        
        WARNING_MESSAGE("interpolateKeyframes : animations have different number of expected bones.");
        return;
    }

    if (t1 < 0 && t2 < 0)
    {
        WARNING_MESSAGE("interpolateKeyframes : both animations have negative time.");
        return;
    }

    if (t2 < 0)
        return animA->getCurrentFrames(t1, lt1, currentKeyframeIndexA, data);

    if (t1 < 0)
        return animB->getCurrentFrames(t2, lt2, currentKeyframeIndexB, data);

    if (t1 > animA->length && t2 > animB->length)
    {
        WARNING_MESSAGE("interpolateKeyframes : both animations have time greater than their length.");
        return;
    }

    // return animA->getCurrentFrames(t1, lt2, currentKeyframeIndexA, data);

    
    /* TODO : figure out what luna meant when making those 2 conditions*/
    if (t1 > animA->length)
        return animB->getCurrentFrames(t2, lt1, currentKeyframeIndexB, data);

    if (t2 > animB->length)
        return animA->getCurrentFrames(t1, lt2, currentKeyframeIndexA, data);

    // std::vector<keyframeData> keyframesA;
    animA->getCurrentFrames(t1, lt1, currentKeyframeIndexA, data);
    static std::vector<keyframeData> keyframesB;
    animB->getCurrentFrames(t2, lt2, currentKeyframeIndexB, keyframesB);

    for (uint i = 0; i < data.size(); i++)
    {
        keyframeData &dataA = data[i];
        keyframeData &dataB = keyframesB[i];

        data[i].translation = glm::mix(dataA.translation, dataB.translation, a);
        data[i].rotation = glm::slerp(dataA.rotation, dataB.rotation, a);
        data[i].scale = glm::mix(dataA.scale, dataB.scale, a);
    }

    return;
}