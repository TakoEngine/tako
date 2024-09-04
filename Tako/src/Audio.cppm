module;
#include "Utility.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <array>
export module Tako.Audio;

import Tako.Assets;

namespace tako
{
	export class AudioClip
	{
	public:
	private:
		AudioClip();
		ma_sound sound;
		friend class Audio;
	};

	export class Audio
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


namespace tako
{
	AudioClip::AudioClip()
	{
	}

	Audio::Audio()
	{
	}

	void Audio::Init()
	{
		ma_result result;
		result = ma_engine_init(NULL, &m_engine);
		ASSERT(result == MA_SUCCESS);
	}

	AudioClip* Audio::Load(const char *file)
	{
		AudioClip* clip = new AudioClip();
		auto result = ma_sound_init_from_file(&m_engine, Assets::GetAssetPath(file).c_str(), 0, nullptr, nullptr, &clip->sound);
		ASSERT(result == MA_SUCCESS);
		return clip;
	}

	void Audio::Play(AudioClip* clip, bool looping)
	{
		ma_sound_set_looping(&clip->sound, looping);
		if (looping)
		{
			ma_sound_set_volume(&clip->sound, 0.6f);
		}
		auto result = ma_sound_start(&clip->sound);
		ASSERT(result == MA_SUCCESS);
	}

	void Audio::Play(const char *soundFile)
	{
		auto result = ma_engine_play_sound(&m_engine, Assets::GetAssetPath(soundFile).c_str(), nullptr);
		ASSERT(result == MA_SUCCESS);
	}

}
