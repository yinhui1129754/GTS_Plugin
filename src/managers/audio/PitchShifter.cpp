
#include "managers/audio/PitchShifter.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "utils/debug.hpp"
#include "utils/av.hpp"
#include "profiler.hpp"
#include "events.hpp"
#include "spring.hpp"
#include "timer.hpp"

#include "Config.hpp"

#include "node.hpp"

#include <vector>
#include <string>

using namespace RE;
using namespace Gts;

namespace Gts {
	 void ShiftAudioFrequency() {
		auto enable = Persistent::GetSingleton().edit_voice_frequency;
		if (!enable) {
			return;
		}
		for (auto tiny: find_actors()) {
			if (tiny) {
				if (tiny->formID != 0x14) {
					auto ai = tiny->GetActorRuntimeData().currentProcess;
					if (ai) {
						auto high = ai->high;
						if (high) {
							auto Audio_1 = high->soundHandles[0];
							auto Audio_2 = high->soundHandles[1];
							auto Audio_3 = high->soundHandles[2];

							float scale = get_visual_scale(tiny) / get_natural_scale(tiny, false) / game_getactorscale(tiny);

							float volume = std::clamp(scale + 0.5f, 0.35f, 1.0f);

							float size = (scale * 0.20) + 0.8;
							float frequence = (1.0 / size) / (1.0 * size);

							

							auto config = Config::GetSingleton().GetVoice();
							float config_param = config.GetVoiceFrequency();

							float freq_high = 1.0 / std::clamp(config_param, 1.0f, 10.0f);

							log::info("Freq high: {}", freq_high);
							float freq_low = 1.5;

							float freq = std::clamp(frequence, freq_high, freq_low);
							// < 1  = deep voice, below 0.5 = audio bugs out, not recommended
							// > 1 = mouse-like voice, not recommended to go above 1.5	

							if (Audio_1.soundID != BSSoundHandle::kInvalidID) {
								Audio_1.SetFrequency(freq);
								Audio_1.SetVolume(volume);
							}
							if (Audio_2.soundID != BSSoundHandle::kInvalidID) {
								Audio_2.SetFrequency(freq);
								Audio_2.SetVolume(volume);
							}
							if (Audio_3.soundID != BSSoundHandle::kInvalidID) {
								Audio_3.SetFrequency(freq);
								Audio_3.SetVolume(volume);
							}
						}
					}
				}
			}
		}
	}
}
