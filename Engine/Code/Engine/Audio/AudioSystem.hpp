#pragma once

//---------------------------------------------------------------------------
#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library


//---------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//---------------------------------------------------------------------------
typedef unsigned int SoundID;
typedef void* AudioChannelHandle;
const unsigned int MISSING_SOUND_ID = 0xffffffff;


/////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	virtual ~AudioSystem();

	SoundID CreateOrGetSound(const std::string& soundFileName);
	AudioChannelHandle GetChannelForChannelID(int channelID);
	AudioChannelHandle PlaySound(SoundID soundID, float volumeLevel = 1.f, AudioChannelHandle prefferedChannel = nullptr);
	void StopChannel(AudioChannelHandle channel);
	void Update();

protected:
	void InitializeFMOD();
	void ShutDownFMOD();
	void ValidateResult(FMOD_RESULT result);

protected:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
};

