#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>

using namespace glm;

/* Weird debug functions, source : 
    https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/ 
*/
#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)
#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device);
bool check_al_errors(const std::string& filename, const std::uint_fast32_t line);

template<typename alcFunction, typename... Params>
auto alcCallImpl(const char* filename, 
                 const std::uint_fast32_t line, 
                 alcFunction function, 
                 ALCdevice* device, 
                 Params... params)
->typename std::enable_if_t<std::is_same_v<void,decltype(function(params...))>,bool>
{
    function(std::forward<Params>(params)...);
    return check_alc_errors(filename,line,device);
}

template<typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char* filename,
                 const std::uint_fast32_t line,
                 alcFunction function,
                 ReturnType& returnValue,
                 ALCdevice* device, 
                 Params... params)
->typename std::enable_if_t<!std::is_same_v<void,decltype(function(params...))>,bool>
{
    returnValue = function(std::forward<Params>(params)...);
    return check_alc_errors(filename,line,device);
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename,
    const std::uint_fast32_t line,
    alFunction function,
    Params... params)
    ->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename,
    const std::uint_fast32_t line,
    alFunction function,
    Params... params)
    ->typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_al_errors(filename, line);
}


/***********************************************************************************************/

extern ALCdevice* openALDevice;
extern ALCcontext* openALContext;

class AudioSource
{
    private : 
        ALuint handle;
        ALint state = AL_PAUSED;

        /*
            AudioSources are non cloneable and non assignables
        */
        AudioSource& operator=(const AudioSource& other) = delete;
        AudioSource(const AudioSource& other) = delete;

    public :    
        AudioSource();
        ~AudioSource();
        AudioSource& play();
        AudioSource& pause();
        AudioSource& loop(bool doLoop = true);
        AudioSource& updateState();
        AudioSource& setBuffer(ALint buffer);
        AudioSource& setPosition(vec3 p);
        AudioSource& setVelocity(vec3 v);
        AudioSource& setPitch(ALfloat pitch);
        AudioSource& setGain(ALfloat gain);

        ALuint getHandle();
};

/*
    TODO : Add streaming audio : 
    https://indiegamedev.net/2020/02/25/the-complete-guide-to-openal-with-c-part-2-streaming-audio/

*/
class AudioFile
{
    private : 
        ALuint handle;
        ALenum format;

    public : 
        void loadOGG(const std::string &filePath);
        ALuint getHandle();
};



#endif