#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "rays/raycast.hpp"
#include "scale/scale.hpp"


using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    const std::string_view Rfinger = "NPC R Finger12 [RF12]";
	const std::string_view Lfinger = "NPC L Finger12 [LF12]";

    void Finger_DoSounds(Actor* giant, std::string_view node_name, float mult) {
		NiAVObject* node = find_node(giant, node_name);
		if (node) {
			float scale = get_visual_scale(giant) * 0.72;
			DoCrawlingSounds(giant, scale * mult, node, FootEvent::Left);
		}
	}

	void Finger_ApplyVisuals(Actor* giant, std::string_view node_name, float threshold, float multiplier) {
		NiAVObject* node = find_node(giant, node_name);
		if (node) {
			float min_scale = 3.5 * threshold;
			float scale = get_visual_scale(giant);
			if (HasSMT(giant)) {
				scale += 2.6;
				multiplier *= 0.33;
			}
			if (scale >= threshold && !giant->AsActorState()->IsSwimming()) {
				NiPoint3 node_location = node->world.translate;

				NiPoint3 ray_start = node_location + NiPoint3(0.0, 0.0, meter_to_unit(-0.05*scale)); // Shift up a little
				NiPoint3 ray_direction(0.0, 0.0, -1.0);
				bool success = false;
				float ray_length = meter_to_unit(std::max(1.05*scale, 1.05));
				NiPoint3 explosion_pos = CastRay(giant, ray_start, ray_direction, ray_length, success);

				if (!success) {
					explosion_pos = node_location;
					explosion_pos.z = giant->GetPosition().z;
				}
				if (giant->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects")) {
					SpawnParticle(giant, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), explosion_pos, (scale * multiplier) * 1.8, 7, nullptr);
				}
				if (giant->formID != 0x14 && Runtime::GetBool("NPCSizeEffects")) {
					SpawnParticle(giant, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), explosion_pos, (scale * multiplier) * 1.8, 7, nullptr);
				}
			}
		}
	}
    void Finger_DoDamage(Actor* giant, bool Right, float Radius, float Damage, float CrushMult, float ShrinkMult) {
		DamageSource source = DamageSource::RightFinger;
		std::string_view NodeLookup = Rfinger;
		if (!Right) {
			source = DamageSource::LeftFinger;
			NodeLookup = Lfinger;
		}

		NiAVObject* node = find_node(giant, NodeLookup);

		ApplyFingerDamage(giant, Radius, Damage, node, 50, 0.10, CrushMult, -0.034 * ShrinkMult, source);
	}

    ////////////////////////////////////////////////////////////////////
    /////// Events
    ///////////////////////////////////////////////////////////////////

    void GTS_Sneak_FingerGrind_CameraOn_R(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::Finger_Right);
	};  

    void GTS_Sneak_FingerGrind_CameraOn_L(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::Finger_Left);
	};  

	void GTS_Sneak_FingerGrind_Impact_R(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_Impact, Damage_Sneak_FingerGrind_Impact, 2.8, 1.2);
		Finger_DoSounds(&data.giant, Rfinger, 1.0);
		Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 1.0);

		DrainStamina(&data.giant, "StaminaDrain_FingerGrind", "DestructionBasics", true, 0.8);
	};
    void GTS_Sneak_FingerGrind_Impact_L(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, false, Radius_Sneak_FingerGrind_Impact, Damage_Sneak_FingerGrind_Impact, 2.8, 1.2);
		Finger_DoSounds(&data.giant, Lfinger, 1.0);
		Finger_ApplyVisuals(&data.giant, Lfinger, 2.6, 1.0);

		DrainStamina(&data.giant, "StaminaDrain_FingerGrind", "DestructionBasics", true, 0.8);
	};

	void GTS_Sneak_FingerGrind_Rotation_R(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 3.2, 0.8);
		Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 0.85);
	};   
    void GTS_Sneak_FingerGrind_Rotation_L(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, false, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 3.2, 0.8);
		Finger_ApplyVisuals(&data.giant, Lfinger, 2.6, 0.85);
	};   

	void GTS_Sneak_FingerGrind_Finisher_R(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_Finisher, Damage_Sneak_FingerGrind_Finisher, 2.4, 3.0);
        Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 1.25);
		Finger_DoSounds(&data.giant, Rfinger, 1.5);
        StopStaminaDrain(&data.giant);	
	};
    void GTS_Sneak_FingerGrind_Finisher_L(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, false, Radius_Sneak_FingerGrind_Finisher, Damage_Sneak_FingerGrind_Finisher, 2.4, 3.0);
        Finger_ApplyVisuals(&data.giant, Lfinger, 2.6, 1.25);
		Finger_DoSounds(&data.giant, Lfinger, 1.5);
        StopStaminaDrain(&data.giant);
		
	};

	void GTS_Sneak_FingerGrind_CameraOff_R(AnimationEventData& data) {TrackMatchingHand(&data.giant, CrawlEvent::RightHand, false);}
    void GTS_Sneak_FingerGrind_CameraOff_L(AnimationEventData& data) {TrackMatchingHand(&data.giant, CrawlEvent::LeftHand, false);}

}

namespace Gts {
    void Animation_SneakSlam_FingerGrind::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOn_R", "Sneak", GTS_Sneak_FingerGrind_CameraOn_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOn_L", "Sneak", GTS_Sneak_FingerGrind_CameraOn_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Impact_R", "Sneak", GTS_Sneak_FingerGrind_Impact_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Impact_L", "Sneak", GTS_Sneak_FingerGrind_Impact_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Rotation_R", "Sneak", GTS_Sneak_FingerGrind_Rotation_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Rotation_L", "Sneak", GTS_Sneak_FingerGrind_Rotation_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Finisher_R", "Sneak", GTS_Sneak_FingerGrind_Finisher_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Finisher_L", "Sneak", GTS_Sneak_FingerGrind_Finisher_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOff_R", "Sneak", GTS_Sneak_FingerGrind_CameraOff_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOff_L", "Sneak", GTS_Sneak_FingerGrind_CameraOff_L);
    }

	void Animation_SneakSlam_FingerGrind::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Tiny_Finger_Impact_S", "Sneak", "GTSBEH_T_Slam_Start");
	}

    void TrackMatchingHand(Actor* giant, CrawlEvent kind, bool enable) {
        if (kind == CrawlEvent::RightHand) {
            ManageCamera(giant, enable, CameraTracking::Hand_Right);
        } else if (kind == CrawlEvent::LeftHand) {
            ManageCamera(giant, enable, CameraTracking::Hand_Left);
        }
    }

    void StopStaminaDrain(Actor* giant) {
		DrainStamina(giant, "StaminaDrain_StrongSneakSlam", "DestructionBasics", false, 2.2);
		DrainStamina(giant, "StaminaDrain_FingerGrind", "DestructionBasics", false, 0.8);
		DrainStamina(giant, "StaminaDrain_SneakSlam", "DestructionBasics", false, 1.4);
	}
}