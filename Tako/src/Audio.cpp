#include "Audio.hpp"
#ifdef TAKO_OPENAL
#include "NumberTypes.hpp"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include "Utility.hpp"


#define SAMPLE_FORMAT   ma_format_f32
#define CHANNEL_COUNT   2
#define SAMPLE_RATE     48000
#endif


namespace tako
{
	AudioClip::AudioClip(const char* file)
	{
#ifdef TAKO_OPENAL
		drwav wavFile;
		drwav_init_file(&wavFile, file, nullptr);

		auto frames = wavFile.totalPCMFrameCount;
		auto channels = wavFile.channels;
		size_t size = frames * channels * sizeof(drwav_int16);
		auto sampleBuffer = static_cast<drwav_int16*>(malloc(size));
		drwav_read_pcm_frames_s16(&wavFile, frames, sampleBuffer);
		alGenBuffers(1, &m_buffer);
		auto format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		alBufferData(m_buffer, format, sampleBuffer, size, wavFile.sampleRate);

		free(sampleBuffer);
		drwav_uninit(&wavFile);
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

	void Audio::Play(AudioClip& clip)
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
