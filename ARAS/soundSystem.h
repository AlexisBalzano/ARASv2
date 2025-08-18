#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>
#include <mmsystem.h>
#include <iostream>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32


namespace Sounds {
		static const inline std::string completionSound = "ressources/sounds/Completion.wav";
}

class SoundPlayer {
public:
	SoundPlayer() = default;
	~SoundPlayer() = default;

	void playSound(const std::string& filePath);
	void stopAllSounds();
};

inline void SoundPlayer::playSound(const std::string& filePath) {
	if (PlaySoundA(filePath.c_str(), NULL, SND_FILENAME | SND_ASYNC)) {
		std::cout << "Playing sound: " << filePath << std::endl;
	} else {
		std::cerr << "Failed to play sound: " << filePath << std::endl;
	}
}

inline void SoundPlayer::stopAllSounds()
{
	std::cout << "Stopping all sounds." << std::endl;
	PlaySoundA(NULL, NULL, SND_PURGE | SND_ASYNC);
}
