#pragma once
#ifdef TAKO_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
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
		static void Play(AudioClip& clip);
	private:
#ifdef TAKO_OPENAL
		ALCdevice* m_device;
		ALCcontext* m_context;
		static std::array<ALuint, 64> m_sources;
#endif
	};
}

