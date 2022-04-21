#include "Audio.hpp"
#include "FileSystem.hpp"
#include "Utility.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"


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
		std::string path("./Assets");
		path.append(file);
		AudioClip* clip = new AudioClip();
		auto result = ma_sound_init_from_file(&m_engine, path.c_str(), 0, nullptr, nullptr, &clip->sound);
		ASSERT(result == MA_SUCCESS);
		return clip;
	}

	void Audio::Play(AudioClip* clip, bool looping)
	{
		ma_sound_set_looping(&clip->sound, looping);
		auto result = ma_sound_start(&clip->sound);
		ASSERT(result == MA_SUCCESS);
	}

	void Audio::Play(const char *soundFile)
	{
		std::string path("./Assets");
		path.append(soundFile);
		auto result = ma_engine_play_sound(&m_engine, path.c_str(), nullptr);
		ASSERT(result == MA_SUCCESS);
	}

}
