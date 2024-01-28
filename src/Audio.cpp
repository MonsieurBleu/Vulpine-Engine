#include <Audio.hpp>
#include <Utils.hpp>
#include <minivorbis.h>

bool check_alc_errors(const std::string& filename, const std::uint_fast32_t line, ALCdevice* device)
{
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
    {
        std::cerr << TERMINAL_ERROR << "***ERROR*** (" << filename << ": " << line << ")\n" ;
        switch(error)
        {
        case ALC_INVALID_VALUE:
            std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case ALC_INVALID_DEVICE:
            std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
            break;
        case ALC_INVALID_CONTEXT:
            std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
            break;
        case ALC_INVALID_ENUM:
            std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
            break;
        case ALC_OUT_OF_MEMORY:
            std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
            break;
        default:
            std::cerr << "UNKNOWN ALC ERROR: " << error;
        }
        std::cerr << TERMINAL_RESET << std::endl;
        return false;
    }
    return true;
}

bool check_al_errors(const std::string& filename, const std::uint_fast32_t line)
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        std::cerr << TERMINAL_ERROR << "***ERROR*** (" << filename << ": " << line << ")\n" ;
        switch(error)
        {
        case AL_INVALID_NAME:
            std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << TERMINAL_RESET << std::endl;
        return false;
    }
    return true;
}

/*
    Source : https://gist.github.com/tilkinsc/f91d2a74cff62cc3760a7c9291290b29
*/
void AudioFile::loadOGG(const std::string &filePath)
{
    OggVorbis_File vf;
    FILE *fp = fopen(filePath.c_str(), "rb");
    short* pcmout = nullptr;
    size_t data_len = 0;
    vorbis_info *vi = nullptr;

    if(!fp)
    {
        std::cerr 
        << TERMINAL_ERROR << "Error loading audio file : "
        << TERMINAL_FILENAME << filePath 
        << TERMINAL_ERROR << "\n";
        perror("\tloadOGG");
        std::cerr << TERMINAL_RESET;
        fclose(fp);
        goto exit;
    }

    if(!alCall(alGenBuffers, (ALsizei)1, &handle))
    {
        std::cerr
        << TERMINAL_ERROR << "Failed to generate sound buffer\n"
        << TERMINAL_RESET;
        goto exit;
    }

    handleRef = std::make_shared<ALuint>(handle);

    if(ov_open_callbacks(fp, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
    {
        std::cerr
        << TERMINAL_ERROR << "Error reading audio file :"
        << TERMINAL_FILENAME << filePath 
        << TERMINAL_ERROR << "\nThis file is not a valid OggVobris file!\n";
        goto exit;
    }
    
    vi = ov_info(&vf, -1);
    format = vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    data_len = ov_pcm_total(&vf, -1) * vi->channels * 2;
    pcmout = (short *)new char[data_len];

    // fill pcmout buffer with ov_read data samples
    // you can't just slap data_len in place of 4096, it doesn't work that way
    // 0 is endianess, 0 for little, 1 for big
    // 2 is the data type short's size, mine is 2
    // 1 is the signedness you want, I want short not unsigned short (for openal) so 1
    for (size_t size = 0, offset = 0, sel = 0;
            (size = ov_read(&vf, (char*) pcmout + offset, 4096, 0, 2, 1, (int*) &sel)) != 0;
            offset += size) {
        if(size < 0)
            puts("Faulty ogg file :o"); // use https://xiph.org/vorbis/doc/vorbisfile/ov_read.html for handling enums
    }

    if(!alCall(alBufferData, handle, format, pcmout, data_len, vi->rate))
    {
        std::cerr
        << TERMINAL_ERROR << "Failed to send audio information buffer to OpenAL!"
        << TERMINAL_RESET;
        goto exit;
    }

    delete pcmout;
	fclose(fp);
	ov_clear(&vf);
	return;

exit:
	delete pcmout;
	fclose(fp);
	ov_clear(&vf);
    return;
}   

ALuint AudioFile::getHandle(){return handle;};

AudioFile::~AudioFile()
{
    if(handle && handleRef.use_count() == 1)
        alCall(alDeleteBuffers, 1, &handle);
}

AudioSource::AudioSource()
{
    // alCall(alGenSources, 1, &handle);
}

AudioSource::~AudioSource()
{
    if(handle && handleRef.use_count() == 1)
    alCall(alDeleteSources, 1, &handle);
}

AudioSource& AudioSource::generate()
{
    if(!handle)
        alCall(alGenSources, 1, &handle);
    return *this;
}

AudioSource& AudioSource::play() {alCall(alSourcePlay, handle); return *this;}

AudioSource& AudioSource::pause() {alCall(alSourcePause, handle); return *this;}

AudioSource& AudioSource::loop(bool doLoop) {alCall(alSourcei, handle, AL_LOOPING, doLoop); return *this;}

AudioSource& AudioSource::updateState() {alCall(alGetSourcei, handle, AL_SOURCE_STATE, &state); return *this;}

AudioSource& AudioSource::setBuffer(ALint buffer) {alCall(alSourcei, handle, AL_BUFFER, buffer); return *this;}

AudioSource& AudioSource::setPosition(vec3 p) {alCall(alSource3f, handle, AL_POSITION, p.x, p.y, p.z); return *this;}

AudioSource& AudioSource::setVelocity(vec3 v) {alCall(alSource3f, handle, AL_VELOCITY, v.x, v.y, v.z); return *this;}

AudioSource& AudioSource::setPitch(ALfloat pitch) {alCall(alSourcef, handle, AL_PITCH, pitch); return *this;}

AudioSource& AudioSource::setGain(ALfloat gain) {alCall(alSourcef, handle, AL_GAIN, gain); return *this;}
