#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Sneak_Slam.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "rays/raycast.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view Rfinger = "NPC R Finger12 [RF12]";
	const std::string_view Lfinger = "NPC L Finger12 [LF12]";

	void Finger_StartShrinkTask(Actor* giant, bool Right, float Radius, float Damage, float CrushMult) {
		auto gianthandle = giant->CreateRefHandle();

		std::string name = std::format("FingerShrink_{}", giant->formID);

		DamageSource source = DamageSource::RightFinger;
		std::string_view NodeLookup = Rfinger;
		if (!Right) {
			source = DamageSource::LeftFinger;
			NodeLookup = Lfinger;
		}

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();

			if (IsFootGrinding(giantref)) {
				NiAVObject* node = find_node(giantref, NodeLookup);
				ApplyFingerDamage(giantref, Radius, Damage, node, 200, 0.05, CrushMult, -0.0004, source);
				return true;
			}
			
			return false;
		});
	}

	void CheckForFingerGrind(Actor* giant, CrawlEvent Event, bool right, std::string_view P2) {
		// Purposed of this task is to check for Finger Grind one frame later
		// So game will detect that actor is Dead - no finger grind will happen
		std::string taskname = std::format("FingerGrindCheck_{}_{}", giant->formID, P2);
		ActorHandle giantHandle = giant->CreateRefHandle();
		TaskManager::RunOnce(taskname, [=](auto& update) {
			if (!giantHandle) {
				return;
			}
			auto giantRef = giantHandle.get().get();
			FingerGrindCheck(giantRef, Event, right, Radius_Sneak_HandSlam);
			Finger_StartShrinkTask(giantRef, right, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 3.0);
		});
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// E V E N T S
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTS_Sneak_Slam_Raise_Arm_R(AnimationEventData& data) {
		Utils_UpdateHighHeelBlend(&data.giant, false);
		TrackMatchingHand(&data.giant, CrawlEvent::RightHand, true);
		DrainStamina(&data.giant, "StaminaDrain_SneakSlam", "DestructionBasics", true, 1.4);
	}
	void GTS_Sneak_Slam_Raise_Arm_L(AnimationEventData& data) {
		Utils_UpdateHighHeelBlend(&data.giant, false);
		TrackMatchingHand(&data.giant, CrawlEvent::LeftHand, true);
		DrainStamina(&data.giant, "StaminaDrain_SneakSlam", "DestructionBasics", true, 1.4);
	}

	void GTS_Sneak_Slam_Lower_Arm_R(AnimationEventData& data) {};
	void GTS_Sneak_Slam_Lower_Arm_L(AnimationEventData& data) {};

	void GTS_Sneak_Slam_Impact_R(AnimationEventData& data) {
		float scale = get_visual_scale(&data.giant);
		DoCrawlingFunctions(&data.giant, scale, 0.75, Damage_Sneak_HandSlam, CrawlEvent::RightHand, "RightHandRumble", 0.80, Radius_Sneak_HandSlam, 1.35, DamageSource::HandSlamRight);
		CheckForFingerGrind(&data.giant, CrawlEvent::RightHand, true, "RH");
		// ^ Also starts finger DOT damage
	};
	void GTS_Sneak_Slam_Impact_L(AnimationEventData& data) {
		float scale = get_visual_scale(&data.giant);
		DoCrawlingFunctions(&data.giant, scale, 0.75, Damage_Sneak_HandSlam, CrawlEvent::LeftHand, "LeftHandRumble", 0.80, Radius_Sneak_HandSlam, 1.35, DamageSource::HandSlamRight);
		CheckForFingerGrind(&data.giant, CrawlEvent::LeftHand, false, "LH");	
		// ^ Also starts finger DOT damage
	};
	
	 
	void GTS_Sneak_Slam_Cam_Off_R(AnimationEventData& data) {
		TrackMatchingHand(&data.giant, CrawlEvent::RightHand, false);
		StopStaminaDrain(&data.giant);
	};        
	void GTS_Sneak_Slam_Cam_Off_L(AnimationEventData& data) {
		TrackMatchingHand(&data.giant, CrawlEvent::LeftHand, false);
		StopStaminaDrain(&data.giant);
	};
}


namespace Gts {
    
    void Animation_SneakSlam::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_Sneak_Slam_Raise_Arm_R", "Sneak", GTS_Sneak_Slam_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Slam_Raise_Arm_L", "Sneak", GTS_Sneak_Slam_Raise_Arm_L);

		AnimationManager::RegisterEvent("GTS_Sneak_Slam_Impact_R", "Sneak", GTS_Sneak_Slam_Impact_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Slam_Impact_L", "Sneak", GTS_Sneak_Slam_Impact_L);

		AnimationManager::RegisterEvent("GTS_Sneak_Slam_Cam_Off_R", "Sneak", GTS_Sneak_Slam_Cam_Off_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Slam_Cam_Off_L", "Sneak", GTS_Sneak_Slam_Cam_Off_L);
    }
}