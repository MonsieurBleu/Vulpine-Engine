#include "Animation.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Utils.hpp"

AnimationRef Animation::load(const std::string &filename)
{
    FILE *file = fopen(filename.c_str(), "rb");
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
    fread(&header, sizeof(AnimationFileHeader), 1, file);

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

    std::vector<std::vector<AnimationKeyframeDataMatrix>> keyframes(header.animatedBoneNumber, std::vector<AnimationKeyframeDataMatrix>());
    for (int i = 0; i < header.animatedBoneNumber; i++)
    {
        AnimationBoneData boneData;
        fread(&boneData, sizeof(AnimationBoneData), 1, file);

        keyframes[boneData.boneID].resize(boneData.keyframeNumber);
        fread(keyframes[boneData.boneID].data(), sizeof(AnimationKeyframeDataMatrix), boneData.keyframeNumber, file);
    }

    fclose(file);

    std::string name = header.animationName;
    AnimationRef anim = std::make_shared<Animation>(header.type, name, header.duration, keyframes);
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

    for (int i = 0; i < keyframes.size(); i++)
    {
        if (keyframes[i].size() == 0 || i != 12)
        {
            state[i] = mat4(1);
            continue;
        }

        int j = 0;
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

        std::cout << keyframes[i][j].matrix[0].x << " " << keyframes[i][j].matrix[0].y << " " << keyframes[i][j].matrix[0].z << " " << keyframes[i][j].matrix[0].w << std::endl;
        std::cout << keyframes[i][j].matrix[1].x << " " << keyframes[i][j].matrix[1].y << " " << keyframes[i][j].matrix[1].z << " " << keyframes[i][j].matrix[1].w << std::endl;
        std::cout << keyframes[i][j].matrix[2].x << " " << keyframes[i][j].matrix[2].y << " " << keyframes[i][j].matrix[2].z << " " << keyframes[i][j].matrix[2].w << std::endl;
        std::cout << keyframes[i][j].matrix[3].x << " " << keyframes[i][j].matrix[3].y << " " << keyframes[i][j].matrix[3].z << " " << keyframes[i][j].matrix[3].w << std::endl
                  << std::endl;
    }
}