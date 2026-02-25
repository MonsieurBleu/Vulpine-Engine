#pragma once 

#include "AssetManager.hpp"
#include "Audio.hpp"
#include "Utils.hpp"

#include <glm/glm.hpp>
using namespace glm;

struct AudioClipInfo
{
    const char* clipName;

    bool usePosition = true;
    vec3 position = vec3(0);
    bool useVelocity = true;
    vec3 velocity = vec3(0);
    bool followEntity = false;

    bool loop = false;

    float pitch = 1.0f;
    float gain = 1.0f;
};

struct AudioClip
{
    AudioClipInfo info;
    AudioSource source;
};

struct AudioPlayer
{
    static constexpr size_t maxClipCount = 8;
    AudioClip clips[maxClipCount];
    size_t clipCount;

    // TODO: move this to impl?
    void Play(const AudioClipInfo& clipInfo) { 
        if (clipCount + 1 == maxClipCount)
        {
            WARNING_MESSAGE("Tried to add new clip to audio player that already has too many clips!");
            return;
        }

        AudioClip& clip = clips[clipCount++];

        clip.info = clipInfo;
        clip.source.generate();
        clip.source.setBuffer(Loader<AudioFile>::get(clipInfo.clipName).getHandle());
        clip.source.play();
    }
};