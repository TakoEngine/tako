#pragma once
#ifdef TAKO_OPENAL
#ifdef TAKO_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#endif
#include <array>




namespace tako
{
	class AudioClip
	{
	public:
		AudioClip(const char* file);
	private:
#ifdef TAKO_OPENAL
		ALuint m_buffer;
#endif
		friend class Audio;
	};

	class Audio
	{
	public:
		Audio();
		void Init();
		static void Play(AudioClip& clip, bool looping = false);
	private:
#ifdef TAKO_OPENAL
		ALCdevice* m_device;
		ALCcontext* m_context;
		static std::array<ALuint, 64> m_sources;
#endif
	};
}

