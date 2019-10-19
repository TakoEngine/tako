#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <array>


namespace tako
{
	class AudioClip
	{
	public:
		AudioClip(const char* file);
	private:
		ALuint m_buffer;
		friend class Audio;
	};

	class Audio
	{
	public:
		Audio();
		void Init();
		static void Play(AudioClip& clip);
	private:
		ALCdevice* m_device;
		ALCcontext* m_context;
		static std::array<ALuint, 64> m_sources;
	};
}
