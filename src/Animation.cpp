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

    std::vector<std::vector<AnimationKeyframeDataMatrix>> keyframes(skeleton->getSize(), std::vector<AnimationKeyframeDataMatrix>());
    for (uint i = 0; i < header.animatedBoneNumber; i++)
    {
        AnimationBoneData boneData;
        // fread(&boneData, sizeof(AnimationBoneData), 1, file);
        file.read((char *)&boneData, sizeof(AnimationBoneData));

        keyframes[boneData.boneID].resize(boneData.keyframeNumber);
        // fread(keyframes[boneData.boneID].data(), sizeof(AnimationKeyframeDataMatrix), boneData.keyframeNumber, file);
        file.read((char *)keyframes[boneData.boneID].data(), sizeof(AnimationKeyframeDataMatrix)*boneData.keyframeNumber);
    }

    // fclose(file);
    file.close();

    std::string name = header.animationName;
    AnimationRef anim = std::make_shared<Animation>(header.type, name, header.duration, keyframes, skeleton);
    return anim;
}

void Animation::apply(float time, SkeletonAnimationState &state) const
{
    // just testing for now, ignore time parameter
    if (keyframes.size() != state.size())
    {
        std::cerr << TERMINAL_ERROR
                  << "Animation::apply => state size does not match keyframes size.\n"
                  << "state size: " << state.size() << ", keyframes size: " << keyframes.size() << ".\n"
                  << TERMINAL_RESET;
        return;
    }

    time = mod(time, 30.f);

    for (uint i = 0; i < keyframes.size(); i++)
    {
        if (keyframes[i].size() == 0)
        {
            state[i] = mat4(1);
            continue;
        }

        uint j = 0;
        for (; j < keyframes[i].size(); j++)
        {
            if (keyframes[i][j].time > time)
                break;
        }

        if (j == 0)
        {
            state[i] = keyframes[i][0].matrix;
        }
        else
        {
            state[i] = keyframes[i][j - 1].matrix;
        }
    }
}