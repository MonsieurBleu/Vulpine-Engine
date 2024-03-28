#include "Animation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Utils.hpp"

#include <iostream>
#include <fstream>

AnimationRef Animation::load(SkeletonRef skeleton, const std::string &filename)
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

    std::vector<std::vector<AnimationKeyframeData>> keyframes(skeleton->getSize(), std::vector<AnimationKeyframeData>());
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
    AnimationRef anim = std::make_shared<Animation>(name, header.duration, keyframes, skeleton);
    return anim;
}

std::vector<keyframeData> Animation::getCurrentFrames(float time)
{
    static float lastTime = 0;

    std::vector<keyframeData> data(keyframes.size());

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

        uint nextKeyframeIndex = (currentKeyframeIndex[i] + 1) % keyframes[i].size();

        if (time > keyframes[i][nextKeyframeIndex].time)
        {
            while (time > keyframes[i][nextKeyframeIndex].time)
            {
                currentKeyframeIndex[i] = nextKeyframeIndex;
                nextKeyframeIndex = (currentKeyframeIndex[i] + 1) % keyframes[i].size();

                if (currentKeyframeIndex[i] == 0)
                    break;
            }
        }

        // get the time difference between the two keyframes
        float deltaTime = keyframes[i][nextKeyframeIndex].time - keyframes[i][currentKeyframeIndex[i]].time;

        // get the time difference between the current time and the current keyframe
        float currentTime = time - keyframes[i][currentKeyframeIndex[i]].time;

        // get the interpolation factor
        float factor = currentTime / deltaTime;

        // if (i == 12)
        //     std::cout << "time: " << time << ", currentKeyframeIndex: " << currentKeyframeIndex[i] << ", nextIndex: " << nextKeyframeIndex << ", factor: " << factor << ", deltaTime: " << deltaTime << ", currentTime: " << currentTime << ", keyframes[i][currentKeyframeIndex[i]].time: " << keyframes[i][currentKeyframeIndex[i]].time << "\n";

        // interpolate translation
        vec3 translation = glm::mix(keyframes[i][currentKeyframeIndex[i]].translation, keyframes[i][nextKeyframeIndex].translation, factor);

        // interpolate rotation
        quat rotation = glm::slerp(keyframes[i][currentKeyframeIndex[i]].rotation, keyframes[i][nextKeyframeIndex].rotation, factor);

        // interpolate scale
        vec3 scale = glm::mix(keyframes[i][currentKeyframeIndex[i]].scale, keyframes[i][nextKeyframeIndex].scale, factor);

        data[i] = {translation, rotation, scale};
    }

    return data;
}