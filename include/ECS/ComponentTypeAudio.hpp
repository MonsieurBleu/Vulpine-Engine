#pragma once 

#include "AssetManager.hpp"
#include "Audio.hpp"
#include "Utils.hpp"

#include <glm/glm.hpp>
#include <random>
using namespace glm;

struct AudioClipSettings
{
    const char* clipName; // name of clip to load

    bool usePosition = true;   // if false the sound plays exactly where the listener is
    vec3 position = vec3(0);   // depending on `positionInEntityReferential` either the position in local entity space or world space
    vec3 velocity = vec3(0);   // used as override value if `updateVelocityBasedOnRigidbody` is false

    bool positionInEntityReferential = true; // if true then uses the position as an offset in the entity's referential, otherwise it is played in world space

    bool updateVelocityBasedOnRigidbody = true; // if true will update the sound clip's velocity every frame based on the rigidbody, if false uses the `velocity` value

    bool loop = false; // whether the clip loops, be warned this means you'll have to kill it manually if you want it to end (using something like clip.source.stop())

    float pitch = 1.0f; // the pitch, range is [0.5 - 2.0] according to openAL
    float gain = 1.0f; // the gain, each division or multiplication by two is about +-6dB, range [0.0 - ]
};

struct AudioClip
{
    AudioClipSettings info;
    AudioSource source;
};

struct AudioPlayer
{
    // static sized buffer for clips, if this ever becomes a problem you can probably just use a std::vector or whatever
    static constexpr size_t maxClipCount = 8;
    AudioClip clips[maxClipCount];
    size_t clipCount = 0;


    AudioClip& Play(const AudioClipSettings& clipSettings) { 
        if (clipCount + 1 == maxClipCount)
        {
            WARNING_MESSAGE("Tried to add new clip to audio player that already has too many clips!");
            return clips[0];
        }

        AudioClip& clip = clips[clipCount++];

        clip.info = clipSettings;
        clip.source.generate();
        clip.source.setBuffer(Loader<AudioFile>::get(clipSettings.clipName).getHandle());
        clip.source.play();

        
        clip.source.loop(clip.info.loop);
        clip.source.setPosition(vec3(clip.info.position.z, clip.info.position.y, clip.info.position.x)); // initialized in world space since we don't have access to the component types for the entity position and stuff
        clip.source.setVelocity(clip.info.velocity);
        clip.source.setGain(clip.info.gain);
        clip.source.setPitch(clip.info.pitch);
        return clip;
    }
};

// Creates audio clips at random intervals within a bounding box
// Only works if the entity it's added to also has an audioPlayer component (cause otherwise it can't play audio duh)
struct AudioScatterer
{
    // The random number generator for the scatterer
    static inline thread_local std::mt19937 generator = std::mt19937(time(nullptr));

    // == Parameters ==
    std::vector<std::string> clips; // will pick a random clip in this list
    float meanTime = 1.0; // the mean time between audio clip plays
    float stddev = 0.5; // the standard deviation of the distribution (cf the normal distribution)

    vec3 posOffset = vec3(0.0f); // the position of the box in entity space
    vec3 halfExtents = vec3(0.5f); // the half extents of the box
    // TODO: see if pertinent to add rotation field to the parameters
    // TODO: also see if pertinent to make the bounding box oriented since it's just sounds 
    //       and typically I must imagine that the boxes will either be too small to make a difference or for large ones square enough or not rotating or something idk

    bool restrictToDistanceFromPlayer = false; // if true will only play a sound in a point that is within the volume and under a certain distance from the player
    float maxDistanceFromPlayer = 30.0f; // if `restrictToWithinRadiusFromPlayer` is true then the radius within which the sounds will play

    // the default settings for the audio player
    // the position and clip name field will be overriden
    AudioClipSettings defaultClipSettings; 
    // == Internal fields ==
    float _lastPlayTime = 0.0f;
    float _timeBeforeNextPlay = 0.0f;

    // std::normal_distribution<float> distribution = std::normal_distribution<float>(meanTime, stddev);
};