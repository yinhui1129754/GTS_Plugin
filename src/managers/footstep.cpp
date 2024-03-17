#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "managers/modevent.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "node.hpp"
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	struct VolumeParams {
		float a;
		float k;
		float n;
		float s;
	};

	float volume_function(float scale, const VolumeParams& params) {
		float k = params.k;
		float a = params.a;
		float n = params.n;
		float s = params.s;
		// https://www.desmos.com/calculator/ygoxbe7hjg
		return k*pow(s*(scale-a), n);
	}

	float frequency_function(float scale, const VolumeParams& params) {
		float a = params.a;
		return soft_core(scale, 0.01, 1.0, 1.0, a, 0.0)*0.5+0.5;
	}

	BSSoundHandle get_sound(NiAVObject* foot, const float& scale, BSISoundDescriptor* sound_descriptor, const VolumeParams& params, std::string_view tag, float mult) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {

			float volume = volume_function(scale, params);
			float frequency = frequency_function(scale, params);
			float falloff = Sound_GetFallOff(foot, mult);
			float intensity = volume * falloff;

			if (intensity > 1e-5) {
				// log::trace("  - Playing {} with volume: {}, falloff: {}, intensity: {}", tag, volume, falloff, intensity);
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				result.SetVolume(intensity);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_lFootstep_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("lFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("lFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_lJumpLand_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::JumpLand:
				return Runtime::GetSound("lJumpLand");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlFootstep_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xlFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xlFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlRumble_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xlRumbleL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xlRumbleR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xlRumbleR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlSprint_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xlSprintL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xlSprintR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xlSprintR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xxlFootstep_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xxlFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xxlFootstepR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xxlFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* GetNormalSound(float scale) {
		if (scale == 2.0) {
			return Runtime::GetSound("Normal_x2");
		} else if (scale == 4.0) {
			return Runtime::GetSound("Normal_x4");
		} else if (scale == 8.0) {
			return Runtime::GetSound("Normal_x8");
		} else if (scale == 12.0) {
			return Runtime::GetSound("Normal_x12");
		} else if (scale == 24.0) {
			return Runtime::GetSound("Normal_x24");
		} else if (scale == 48.0) {
			return Runtime::GetSound("Normal_x48");
		} else if (scale == 96.0) {
			return Runtime::GetSound("Normal_x96");
		} else if (scale > 96.0) {
			return Runtime::GetSound("Normal_Mega");
		}
		return nullptr;
	}
	BSISoundDescriptor* GetNormalSound_Jump(float scale) {
		if (scale == 2.0) {
			return Runtime::GetSound("NormalLand_x2");
		} else if (scale == 4.0) {
			return Runtime::GetSound("NormalLand_x4");
		} else if (scale == 8.0) {
			return Runtime::GetSound("NormalLand_x8");
		} else if (scale == 12.0) {
			return Runtime::GetSound("NormalLand_x12");
		} else if (scale == 24.0) {
			return Runtime::GetSound("NormalLand_x24");
		} else if (scale == 48.0) {
			return Runtime::GetSound("NormalLand_x48");
		} else if (scale == 96.0) {
			return Runtime::GetSound("NormalLand_x96");
		} else if (scale > 96.0) {
			return Runtime::GetSound("NormalLand_Mega");
		}
		return nullptr;
	}

	BSISoundDescriptor* GetHHSound_Normal(float scale) {
		if (scale == 2.0) {
			return Runtime::GetSound("HighHeel_x2");
		} else if (scale == 4.0) {
			return Runtime::GetSound("HighHeel_x4");
		} else if (scale == 8.0) {
			return Runtime::GetSound("HighHeel_x8");
		} else if (scale == 12.0) {
			return Runtime::GetSound("HighHeel_x12");
		} else if (scale == 24.0) {
			return Runtime::GetSound("HighHeel_x24");
		} else if (scale == 48.0) {
			return Runtime::GetSound("HighHeel_x48");
		} else if (scale == 96.0) {
			return Runtime::GetSound("HighHeel_x96");
		} else if (scale > 96.0) {
			return Runtime::GetSound("HighHeel_Mega");
		}
		return nullptr;
	}
	BSISoundDescriptor* GetHHSound_Jump(float scale) {
		if (scale == 2.0) {
			return Runtime::GetSound("HighHeelLand_x2");
		} else if (scale == 4.0) {
			return Runtime::GetSound("HighHeelLand_x4");
		} else if (scale == 8.0) {
			return Runtime::GetSound("HighHeelLand_x8");
		} else if (scale == 12.0) {
			return Runtime::GetSound("HighHeelLand_x12");
		} else if (scale == 24.0) {
			return Runtime::GetSound("HighHeelLand_x24");
		} else if (scale == 48.0) {
			return Runtime::GetSound("HighHeelLand_x48");
		} else if (scale == 96.0) {
			return Runtime::GetSound("HighHeelLand_x96");
		} else if (scale > 96.0) {
			return Runtime::GetSound("HighHeelLand_Mega");
		}
		return nullptr;
	}
	BSISoundDescriptor* get_footstep_highheel(const FootEvent& foot_kind, float scale) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return GetHHSound_Normal(scale);
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return GetHHSound_Normal(scale);
				break;
			case FootEvent::JumpLand:
				return GetHHSound_Jump(scale);
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_footstep_normal(const FootEvent& foot_kind, float scale) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return GetNormalSound(scale);
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return GetNormalSound(scale);
				break;
			case FootEvent::JumpLand:
				return GetNormalSound_Jump(scale);
				break;
		}
		return nullptr;
	}
}
namespace Gts {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	std::string FootStepManager::DebugName() {
		return "FootStepManager";
	}

	void FootStepManager::OnImpact(const Impact& impact) {
		if (impact.actor) {
			auto profiler = Profilers::Profile("FootStepSound: OnImpact");
			auto player = PlayerCharacter::GetSingleton();
			auto actor = impact.actor;
			float scale = impact.scale;

			if (actor->formID == 0x14 && HasSMT(actor)) {
				scale *= 2.5;
			}

			float sprint_factor = 1.0;
			bool LegacySounds = Persistent::GetSingleton().legacy_sounds;  // Determine if we should play old pre 2.00 update sounds
			// ^ Currently forced to true: there's no sounds yet.
			bool sprinting = false;
			bool WearingHighHeels = HighHeelManager::IsWearingHH(actor);
			if (scale > 1.2 && !actor->AsActorState()->IsSwimming()) {
				float start_l = 1.2;
				float start_xl = 11.99;
				float start_xlJumpLand= 1.99;
				float start_xxl = 20.0;

				FootEvent foot_kind = impact.kind;

				if (actor->AsActorState()->IsSprinting()) { // Sprinting makes you sound bigger
					sprint_factor = 1.10;
					scale *= sprint_factor;
					start_xl = 9.8;
					start_xxl = 16.0;
					sprinting = true;
				}
				if (actor->AsActorState()->IsWalking()) {
					scale *= 0.75; // Walking makes you sound quieter
				}
				if (actor->IsSneaking()) {
					scale *= 0.60; // Sneaking makes you sound quieter
				}
				if (actor->formID == 0x14 && IsFirstPerson()) { // Footsteps are quieter when in first person
					scale *= 0.70;
				}

				if (foot_kind == FootEvent::JumpLand) {
					scale *= 1.2; // Jumping makes you sound bigger
					start_xl = 7.8;
					start_xxl = 14.0;
				}
				if (Runtime::GetBool("EnableGiantSounds")) {
					for (NiAVObject* foot: impact.nodes) {
						FootStepManager::PlayLegacySounds(foot, foot_kind, scale, start_l, start_xl, start_xxl);
						return; // New soundsa re disabled for now
						if (!LegacySounds && WearingHighHeels) { // Play high heel sounds that will be done someday
							FootStepManager::PlayHighHeelSounds(foot, foot_kind, scale, sprint_factor, sprinting);
							return;
						} else if (!LegacySounds && !WearingHighHeels) { // Play non HH sounds that will be done someday
							FootStepManager::PlayNormalSounds(foot, foot_kind, scale, sprint_factor, sprinting);
							return;
						} else if (LegacySounds) { // Play old sounds
							FootStepManager::PlayLegacySounds(foot, foot_kind, scale, start_l, start_xl, start_xxl);
							return;
						}
					}
				}
			}
		}
	}

	void FootStepManager::PlayLegacySounds(NiAVObject* foot, FootEvent foot_kind, float scale, float start_l, float start_xl, float start_xxl) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		auto profiler = Profilers::Profile("Impact: PlayLegacySounds");
		BSSoundHandle lFootstep    = get_sound(foot, scale, get_lFootstep_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.45,  .n = 0.7, .s = 1.0}, "L Footstep", 1.0);
		BSSoundHandle lJumpLand    = get_sound(foot, scale, get_lJumpLand_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.65,  .n = 0.7, .s = 1.0}, "L Jump", 1.0);

		BSSoundHandle xlFootstep   = get_sound(foot, scale, get_xlFootstep_sounddesc(foot_kind),  VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL: Footstep", 1.0);
		BSSoundHandle xlRumble     = get_sound(foot, scale, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Rumble", 1.0);
		BSSoundHandle xlSprint     = get_sound(foot, scale, get_xlSprint_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Sprint", 1.0);

		BSSoundHandle Footstep_12 = get_sound(foot, scale, get_footstep_highheel(foot_kind, 12),  VolumeParams { .a = 12.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x12 Footstep", 1.8);

		BSSoundHandle xxlFootstepL = get_sound(foot, scale, get_xxlFootstep_sounddesc(foot_kind), VolumeParams { .a = start_xxl,           .k = 0.5,  .n = 0.5, .s = 1.0}, "XXL Footstep", 1.0);
		if (lFootstep.soundID != BSSoundHandle::kInvalidID && scale <= 14.0) {
			lFootstep.Play();
		}
		if (lJumpLand.soundID != BSSoundHandle::kInvalidID) {
			lJumpLand.Play();
		}
		if (xlFootstep.soundID != BSSoundHandle::kInvalidID && scale <= 14.0) {
			xlFootstep.Play();
		}
		if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
			xlRumble.Play();
		}
		if (xlSprint.soundID != BSSoundHandle::kInvalidID && scale <= 14.0) {
			xlSprint.Play();
		}
		if (xxlFootstepL.soundID != BSSoundHandle::kInvalidID && scale <= 14.0) {
			xxlFootstepL.Play();
		}
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) {
			Footstep_12.Play();
		}
	}

	void FootStepManager::PlayHighHeelSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		BSSoundHandle Footstep_1  = get_sound(foot, scale, get_footstep_highheel(foot_kind, 2),  VolumeParams { .a = 1.15,           .k = 0.45,  .n = 0.7, .s = 1.0}, "x1 Footstep", 1.0);
		BSSoundHandle Footstep_2  = get_sound(foot, scale, get_footstep_highheel(foot_kind, 2),  VolumeParams { .a = 2.0,           .k = 0.45,  .n = 0.7, .s = 1.0}, "x2 Footstep", 1.0);
		BSSoundHandle Footstep_4  = get_sound(foot, scale, get_footstep_highheel(foot_kind, 4),  VolumeParams { .a = 4.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x4 Footstep", 1.2);
		BSSoundHandle Footstep_8  = get_sound(foot, scale, get_footstep_highheel(foot_kind, 8),  VolumeParams { .a = 8.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x8 Footstep", 1.6);
		BSSoundHandle Footstep_12 = get_sound(foot, scale, get_footstep_highheel(foot_kind, 12),  VolumeParams { .a = 12.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x12 Footstep", 1.8);
		BSSoundHandle Footstep_24 = get_sound(foot, scale, get_footstep_highheel(foot_kind, 24),  VolumeParams { .a = 24.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x24 Footstep", 2.0);
		BSSoundHandle Footstep_48 = get_sound(foot, scale, get_footstep_highheel(foot_kind, 48),  VolumeParams { .a = 48.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x48 Footstep", 3.0);
		BSSoundHandle Footstep_96 = get_sound(foot, scale, get_footstep_highheel(foot_kind, 96),  VolumeParams { .a = 96.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x96 Footstep", 6.0);
		BSSoundHandle Footstep_Mega = get_sound(foot, scale, get_footstep_highheel(foot_kind, 98),  VolumeParams { .a = 110.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "Mega Footstep", 10.0);

		if (Footstep_1.soundID != BSSoundHandle::kInvalidID) {
			Footstep_1.Play();
		}
		if (Footstep_2.soundID != BSSoundHandle::kInvalidID) {
			Footstep_2.Play();
		}
		if (Footstep_4.soundID != BSSoundHandle::kInvalidID) {
			Footstep_4.Play();
		}
		if (Footstep_8.soundID != BSSoundHandle::kInvalidID) {
			Footstep_8.Play();
		}
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) {
			Footstep_12.Play();
		}
		if (Footstep_24.soundID != BSSoundHandle::kInvalidID) {
			Footstep_24.Play();
		}
		if (Footstep_48.soundID != BSSoundHandle::kInvalidID) {
			Footstep_48.Play();
		}
		if (Footstep_96.soundID != BSSoundHandle::kInvalidID) {
			Footstep_96.Play();
		}
		if (Footstep_Mega.soundID != BSSoundHandle::kInvalidID) {
			Footstep_Mega.Play();
		}
	}

	void FootStepManager::PlayNormalSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting) {
		BSSoundHandle Footstep_1  = get_sound(foot, scale, get_footstep_normal(foot_kind, 2),  VolumeParams { .a = 1.2,           .k = 0.45,  .n = 0.7, .s = 1.0}, "x1 FootstepNormal", 1.0);
		BSSoundHandle Footstep_2  = get_sound(foot, scale, get_footstep_normal(foot_kind, 2),  VolumeParams { .a = 2.0,           .k = 0.45,  .n = 0.7, .s = 1.0}, "x2 FootstepNormal", 1.0);
		BSSoundHandle Footstep_4  = get_sound(foot, scale, get_footstep_normal(foot_kind, 4),  VolumeParams { .a = 4.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x4 FootstepNormal", 1.2);
		BSSoundHandle Footstep_8  = get_sound(foot, scale, get_footstep_normal(foot_kind, 8),  VolumeParams { .a = 8.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x8 FootstepNormal", 1.6);
		BSSoundHandle Footstep_12 = get_sound(foot, scale, get_footstep_normal(foot_kind, 12),  VolumeParams { .a = 12.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x12 FootstepNormal", 1.8);
		BSSoundHandle Footstep_24 = get_sound(foot, scale, get_footstep_normal(foot_kind, 24),  VolumeParams { .a = 24.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x24 FootstepNormal", 2.0);
		BSSoundHandle Footstep_48 = get_sound(foot, scale, get_footstep_normal(foot_kind, 48),  VolumeParams { .a = 48.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x48 FootstepNormal", 3.0);
		BSSoundHandle Footstep_96 = get_sound(foot, scale, get_footstep_normal(foot_kind, 96),  VolumeParams { .a = 96.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x96 FootstepNormal", 6.0);
		BSSoundHandle Footstep_Mega = get_sound(foot, scale, get_footstep_normal(foot_kind, 98),  VolumeParams { .a = 120.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "Mega FootstepNormal", 10.0);

		if (Footstep_1.soundID != BSSoundHandle::kInvalidID) {
			Footstep_1.Play();
		}
		if (Footstep_2.soundID != BSSoundHandle::kInvalidID) {
			Footstep_2.Play();
		}
		if (Footstep_4.soundID != BSSoundHandle::kInvalidID) {
			Footstep_4.Play();
		}
		if (Footstep_8.soundID != BSSoundHandle::kInvalidID) {
			Footstep_8.Play();
		}
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) {
			Footstep_12.Play();
		}
		if (Footstep_24.soundID != BSSoundHandle::kInvalidID) {
			Footstep_24.Play();
		}
		if (Footstep_48.soundID != BSSoundHandle::kInvalidID) {
			Footstep_48.Play();
		}
		if (Footstep_96.soundID != BSSoundHandle::kInvalidID) {
			Footstep_96.Play();
		}
		if (Footstep_Mega.soundID != BSSoundHandle::kInvalidID) {
			Footstep_Mega.Play();
		}
	}
}
