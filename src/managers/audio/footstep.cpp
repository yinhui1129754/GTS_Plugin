#include "managers/audio/AudioObtainer.hpp"
#include "managers/audio/footstep.hpp"
#include "managers/highheel.hpp"
#include "managers/modevent.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "data/persistent.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	const float limitless = 0.0f;
	const float limit_x2 = 2.0f;
	const float limit_x4 = 4.0f;
	const float limit_x8 = 8.0f;
	const float limit_x12 = 12.0f;
	const float limit_x14 = 14.0f;
	const float limit_x24 = 24.0f;
	const float limit_x48 = 48.0f;
	const float limit_x96 = 96.0f;
	const float limit_mega = 106.0f;

	BSSoundHandle get_sound(NiAVObject* foot, const float& scale, const float& scale_limit, BSISoundDescriptor* sound_descriptor, const VolumeParams& params, std::string_view tag, float mult) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		
		if (sound_descriptor && foot && audio_manager) {

			float volume = volume_function(scale, params);
			float frequency = frequency_function(scale, params);
			float falloff = Sound_GetFallOff(foot, mult);
			float intensity = volume * falloff;
			if (scale_limit > 0.02 && scale > scale_limit) {
				return result; // Return empty sound in that case
			}
			if (intensity > 0.05) {
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
			if (!impact.actor->Is3DLoaded()) {
				return;
			} 
			if (!impact.actor->GetCurrent3D()) {
				return;
			}
			auto profiler = Profilers::Profile("FootStepSound: OnImpact");
			auto player = PlayerCharacter::GetSingleton();
			auto actor = impact.actor;
			float scale = impact.scale;

			if (actor->formID == 0x14 && HasSMT(actor)) {
				scale *= 2.5;
			}

			float sprint_factor = 1.0;
			bool LegacySounds = Persistent::GetSingleton().legacy_sounds;  // Determine if we should play old pre 2.00 update sounds
			// ^ Currently forced to true: there's not a lot of sounds yet.
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
					scale *= 0.80; // Walking makes you sound quieter
				}
				if (actor->IsSneaking()) {
					scale *= 0.70; // Sneaking makes you sound quieter
				}
				/*if (actor->formID == 0x14 && IsFirstPerson()) { // Footsteps are quieter when in first person
					scale *= 0.80;
				}*/

				if (foot_kind == FootEvent::JumpLand) {
					scale *= 1.2; // Jumping makes you sound bigger
					start_xl = 7.8;
					start_xxl = 14.0;
				}

				scale *= 1.0 + (Potion_GetMightBonus(actor) * 0.33);

				if (Runtime::GetBool("EnableGiantSounds")) {
					for (NiAVObject* foot: impact.nodes) {
						if (!foot) {
							return;
						}
						FootStepManager::PlayLegacySounds(foot, foot_kind, scale, start_l, start_xl, start_xxl);
						return; // New sounds are disabled for now

						if (!LegacySounds) { // Use new sounds that we've commissioned
							if (WearingHighHeels) { // Play high heel sounds that are being slowly worked on
								FootStepManager::PlayHighHeelSounds(foot, foot_kind, scale, sprint_factor, sprinting);
								return;
							} else { // Play non high heel sounds that are being slowly worked on
								FootStepManager::PlayNormalSounds(foot, foot_kind, scale, sprint_factor, sprinting);
								return;
							}
						} else { // Else Play old sounds
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

		BSSoundHandle lFootstep    = get_sound(foot, scale, limit_x14, get_lFootstep_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.45,  .n = 0.7, .s = 1.0}, "L Footstep", 1.0);
		BSSoundHandle xlFootstep   = get_sound(foot, scale, limit_x14, get_xlFootstep_sounddesc(foot_kind),  VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL: Footstep", 1.0);
		BSSoundHandle xxlFootstep = get_sound(foot, scale, limit_x14, get_xxlFootstep_sounddesc(foot_kind), VolumeParams { .a = start_xxl,           .k = 0.50,  .n = 0.5, .s = 1.0}, "XXL Footstep", 1.0);
		// These stop to appear at x14
		BSSoundHandle lJumpLand    = get_sound(foot, scale, limitless, get_lJumpLand_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.65,  .n = 0.7, .s = 1.0}, "L Jump", 1.0);

		BSSoundHandle xlRumble     = get_sound(foot, scale, limitless, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Rumble", 1.0);
		//BSSoundHandle xlSprint     = get_sound(foot, scale, get_xlSprint_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Sprint", 1.0);
        //  ^ Same normal sounds but a tiny bit louder: 319060: Sound\fx\GTS\Effects\Footsteps\Original\Movement
		BSSoundHandle Footstep_12 = get_sound(foot, scale, limit_x24, get_footstep_highheel(foot_kind, 12),  VolumeParams { .a = 12.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x12 Footstep", 1.8);
		// Stops at x24
		BSSoundHandle Footstep_24 = get_sound(foot, scale, limitless, get_footstep_highheel(foot_kind, 24),  VolumeParams { .a = 20.0,          .k = 0.30, .n = 0.55, .s = 1.0}, "x24 Footstep", 2.6);
		// Always plays past x22.0

		if (lFootstep.soundID != BSSoundHandle::kInvalidID) { // x1.2: 
			// 1E93AB: Sound\fx\GTS\Effects\Footsteps\Original\Movement\Footstep 1 - 4.wav
			lFootstep.Play();
		}
		if (xlFootstep.soundID != BSSoundHandle::kInvalidID) { 
			// 271EF4: Sound\fx\GTS\Foot\Effects  (Stone sounds)
			xlFootstep.Play();
		}
		if (xxlFootstep.soundID != BSSoundHandle::kInvalidID) { 
			// 16FB25: Sound\fx\GTS\Effects\Footsteps\Original\Rumble (Distant foot sounds)
			xxlFootstep.Play();
		}

		if (lJumpLand.soundID != BSSoundHandle::kInvalidID) { // Jump Land audio: 
			// 183F43: Sound\fx\GTS\Effects\Footsteps\Original\Fall
			lJumpLand.Play();
		}
		if (xlRumble.soundID != BSSoundHandle::kInvalidID) { // Rumble when walking at huge scale: 
			// 36A06D: Sound\fx\GTS\Foot\Effects\Rumble1-4.wav
			xlRumble.Play();
		}

		//=================================== Custom Commissioned Sounds =========================================
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) { // x12 Custom audio
			Footstep_12.Play();
		}

		if (Footstep_24.soundID != BSSoundHandle::kInvalidID) { // x24 Custom audio
			Footstep_24.Play();
		}
		
	}

	void FootStepManager::PlayHighHeelSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		// 2024.04.23: Only 2 sets are done for now: x12 and x24 (still wip)
		BSSoundHandle xlRumble     = get_sound(foot, scale, limitless, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = 12.0,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Rumble", 1.0);

		BSSoundHandle Footstep_2  = get_sound(foot, scale, limit_x4, get_footstep_highheel(foot_kind, 2),  VolumeParams { .a = 1.15,           .k = 0.45,  .n = 0.7, .s = 1.0}, "x1 Footstep", 1.0);
		BSSoundHandle Footstep_4  = get_sound(foot, scale, limit_x8, get_footstep_highheel(foot_kind, 4),  VolumeParams { .a = 4.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x4 Footstep", 1.2);
		BSSoundHandle Footstep_8  = get_sound(foot, scale, limit_x12, get_footstep_highheel(foot_kind, 8),  VolumeParams { .a = 8.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x8 Footstep", 1.6);
		BSSoundHandle Footstep_12 = get_sound(foot, scale, limit_x24, get_footstep_highheel(foot_kind, 12),  VolumeParams { .a = 12.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x12 Footstep", 1.8);
		BSSoundHandle Footstep_24 = get_sound(foot, scale, limit_x48, get_footstep_highheel(foot_kind, 24),  VolumeParams { .a = 22.0,          .k = 0.30, .n = 0.55, .s = 1.0}, "x24 Footstep", 2.0);
		BSSoundHandle Footstep_48 = get_sound(foot, scale, limit_x96, get_footstep_highheel(foot_kind, 48),  VolumeParams { .a = 48.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x48 Footstep", 3.0);
		BSSoundHandle Footstep_96 = get_sound(foot, scale, limit_mega, get_footstep_highheel(foot_kind, 96),  VolumeParams { .a = 96.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x96 Footstep", 6.0);
		BSSoundHandle Footstep_Mega = get_sound(foot, scale, limitless, get_footstep_highheel(foot_kind, 98),  VolumeParams { .a = 110.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "Mega Footstep", 10.0);
		if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
			xlRumble.Play();
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
		//https://www.desmos.com/calculator/wh0vwgljfl
		// 2024.04.23:  There's no sounds in this set of sounds yet. It will be worked on after high heel sounds will be done
		BSSoundHandle xlRumble     = get_sound(foot, scale, limitless, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = 12.0,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Rumble", 1.0);

		BSSoundHandle Footstep_2  = get_sound(foot, scale, limit_x4, get_footstep_normal(foot_kind, 2),  VolumeParams { .a = 1.15,           .k = 0.45,  .n = 0.7, .s = 1.0}, "x1 Footstep", 1.0);
		BSSoundHandle Footstep_4  = get_sound(foot, scale, limit_x8, get_footstep_normal(foot_kind, 4),  VolumeParams { .a = 4.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x4 Footstep", 1.2);
		BSSoundHandle Footstep_8  = get_sound(foot, scale, limit_x12, get_footstep_normal(foot_kind, 8),  VolumeParams { .a = 8.0,           .k = 0.45, .n = 0.55, .s = 1.0}, "x8 Footstep", 1.6);
		BSSoundHandle Footstep_12 = get_sound(foot, scale, limit_x24, get_footstep_normal(foot_kind, 12),  VolumeParams { .a = 12.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x12 Footstep", 1.8);
		BSSoundHandle Footstep_24 = get_sound(foot, scale, limit_x48, get_footstep_normal(foot_kind, 24),  VolumeParams { .a = 22.0,          .k = 0.30, .n = 0.55, .s = 1.0}, "x24 Footstep", 2.0);
		BSSoundHandle Footstep_48 = get_sound(foot, scale, limit_x96, get_footstep_normal(foot_kind, 48),  VolumeParams { .a = 48.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x48 Footstep", 3.0);
		BSSoundHandle Footstep_96 = get_sound(foot, scale, limit_mega, get_footstep_normal(foot_kind, 96),  VolumeParams { .a = 96.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "x96 Footstep", 6.0);
		BSSoundHandle Footstep_Mega = get_sound(foot, scale, limitless, get_footstep_normal(foot_kind, 98),  VolumeParams { .a = 110.0,          .k = 0.40, .n = 0.55, .s = 1.0}, "Mega Footstep", 10.0);

		if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
			xlRumble.Play();
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
