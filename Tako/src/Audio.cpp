#include "Audio.hpp"
#include "FileSystem.hpp"
#ifdef TAKO_OPENAL
#include <filesystem>
#include "NumberTypes.hpp"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#include "Utility.hpp"


#define SAMPLE_FORMAT   ma_format_f32
#define CHANNEL_COUNT   2
#define SAMPLE_RATE     48000
#endif


namespace tako
{
#ifdef TAKO_OPENAL
    void InitClip(ALuint* buffer, ALenum format, ALvoid* sampleBuffer, ALsizei size, ALsizei sampleRate)
    {
        alGenBuffers(1, buffer);
        alBufferData(*buffer, format, sampleBuffer, size, sampleRate);
    }

    void LoadWav(ALuint* clipBuffer, tako::U8* buffer, size_t bytesRead)
    {
        drwav wavFile;
        drwav_init_memory(&wavFile, buffer, bytesRead, nullptr);

        auto frames = wavFile.totalPCMFrameCount;
        auto channels = wavFile.channels;
        size_t size = frames * channels * sizeof(drwav_int16);
        auto sampleBuffer = static_cast<drwav_int16*>(malloc(size));
        drwav_read_pcm_frames_s16(&wavFile, frames, sampleBuffer);
        auto format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        InitClip(clipBuffer, format, sampleBuffer, size, wavFile.sampleRate);

        free(sampleBuffer);
        drwav_uninit(&wavFile);
    }

    void LoadMp3(ALuint* clipBuffer, tako::U8* buffer, size_t bytesRead)
    {
        drmp3 mp3File;
        drmp3_init_memory(&mp3File, buffer, bytesRead, nullptr, nullptr);

        auto frames = drmp3_get_pcm_frame_count(&mp3File);
        auto channels = mp3File.channels;
        size_t size = frames * channels * sizeof(drmp3_int16);
        auto sampleBuffer = static_cast<drmp3_int16*>(malloc(size));
        drmp3_read_pcm_frames_s16(&mp3File, frames, sampleBuffer);
        auto format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        InitClip(clipBuffer, format, sampleBuffer, size, mp3File.sampleRate);

        free(sampleBuffer);
        drmp3_uninit(&mp3File);
    }
#endif
	AudioClip::AudioClip(const char* file)
	{
#ifdef TAKO_OPENAL
	    auto bufferSize = FileSystem::GetFileSize(file);
        tako::U8* buffer = new tako::U8[bufferSize];
        size_t bytesRead = 0;
        FileSystem::ReadFile(file, buffer, bufferSize, bytesRead);
        auto extension = std::filesystem::path(file).extension();
        if (extension == ".wav")
        {
            LoadWav(&m_buffer, buffer, bytesRead);
        }
        else if (extension == ".mp3")
        {
            LoadMp3(&m_buffer, buffer, bytesRead);
        }
        else
        {
            LOG_ERR("Audioformat {} not supported", extension);
        }
		free(buffer);
#endif
	}

	Audio::Audio()
	{
	}

	void Audio::Init()
	{
#ifdef TAKO_OPENAL
		m_device = alcOpenDevice(nullptr);
		m_context = alcCreateContext(m_device, nullptr);
		alcMakeContextCurrent(m_context);
		alGenSources(m_sources.size(), &m_sources[0]);
#endif
	}

	void Audio::Play(AudioClip& clip, bool looping)
	{
#ifdef TAKO_OPENAL
		ALuint source;
		for (auto source: m_sources)
		{
			ALint state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			if (state == AL_PLAYING)
			{
				continue;
			}

			alSourcei(source, AL_BUFFER, clip.m_buffer);
            alSourcei(source, AL_LOOPING, looping);
            //Hack:
            if (looping)
            {
                alSourcef(source, AL_GAIN, 0.3f);
            }
			alSourcePlay(source);
			return;
		}
		LOG("No source available!");
#endif
	}
#ifdef TAKO_OPENAL
	std::array<ALuint, 64> Audio::m_sources;
#endif
}
