#pragma once
#include <array>
#include "miniaudio.h"

namespace tako
{
	class AudioClip
	{
	public:
	private:
		AudioClip();
		ma_sound sound;
		friend class Audio;
	};

	class Audio
	{
	public:
		Audio();
		void Init();
		AudioClip* Load(const char* file);
		static void Play(AudioClip* clip, bool looping = false);
		void Play(const char* soundFile);
	private:
		ma_engine m_engine;
	};
}

