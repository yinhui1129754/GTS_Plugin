#pragma once

using namespace RE;
using namespace SKSE;

namespace Gts {
	void PlaySound(BSISoundDescriptor* soundDescriptor, Actor* Receiver, float Volume, float Frequency);

	void PlaySound_Frequency(BSISoundDescriptor* soundDescriptor, Actor* Receiver, float Volume, float Frequency);
}
