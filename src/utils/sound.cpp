#include "utils/sound.hpp"

using namespace RE;
using namespace SKSE;

namespace Gts {
	void PlaySound(BSISoundDescriptor* soundDescriptor, Actor* Receiver, float Volume, float Frequency) {
		if (!soundDescriptor) {
			log::error("Sound invalid");
			return;
		}
		auto audioManager = BSAudioManager::GetSingleton();
		if (!audioManager) {
			log::error("Audio Manager invalid");
			return;
		}
		BSSoundHandle soundHandle;
		bool success = audioManager->BuildSoundDataFromDescriptor(soundHandle, soundDescriptor);
		if (success) {
			//soundHandle.SetFrequency(Frequency);
			soundHandle.SetVolume(Volume);
			NiAVObject* follow = nullptr;
			if (Receiver) {
				NiAVObject* current_3d = Receiver->GetCurrent3D();
				if (current_3d) {
					follow = current_3d;
				}
			}
			soundHandle.SetObjectToFollow(follow);
			soundHandle.Play();
		} else {
			log::error("Could not build sound");
		}
	}

	void PlaySound_Frequency(BSISoundDescriptor* soundDescriptor, Actor* Receiver, float Volume, float Frequency) {
		if (!soundDescriptor) {
			log::error("Sound invalid");
			return;
		}
		auto audioManager = BSAudioManager::GetSingleton();
		if (!audioManager) {
			log::error("Audio Manager invalid");
			return;
		}
		BSSoundHandle soundHandle;
		bool success = audioManager->BuildSoundDataFromDescriptor(soundHandle, soundDescriptor);
		if (success) {
			soundHandle.SetFrequency(Frequency);
			soundHandle.SetVolume(Volume);
			NiAVObject* follow = nullptr;
			if (Receiver) {
				NiAVObject* current_3d = Receiver->GetCurrent3D();
				if (current_3d) {
					follow = current_3d;
				}
			}
			soundHandle.SetObjectToFollow(follow);
			soundHandle.Play();
		} else {
			log::error("Could not build sound");
		}
	}
}
