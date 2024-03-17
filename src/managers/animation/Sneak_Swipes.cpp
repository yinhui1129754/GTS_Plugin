#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Sneak_Swipes.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"


using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace { 
	void TriggerHandCollision_Right(Actor* actor, float power, float crush, float pushpower) {
		std::string name = std::format("SwipeCollide_R_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC R Forearm [RLar]");
			auto Arm = find_node(giant, "NPC R Hand [RHnd]");
			if (Uarm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeRight);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeRight);
			}

			Utils_UpdateHighHeelBlend(giant, false);

			return true;
		});
	}

	void TriggerHandCollision_Left(Actor* actor, float power, float crush, float pushpower) {
		std::string name = std::format("SwipeCollide_L_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC L Forearm [LLar]");
			auto Arm = find_node(giant, "NPC L Hand [LHnd]");
			if (Uarm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeLeft);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeLeft);
			}

			Utils_UpdateHighHeelBlend(giant, false);

			return true;
		});
	}
	void DisableHandCollisions(Actor* actor) {
		std::string name = std::format("SwipeCollide_L_{}", actor->formID);
		std::string name2 = std::format("SwipeCollide_R_{}", actor->formID);
		TaskManager::Cancel(name);
		TaskManager::Cancel(name2);
	}

	void GTS_Sneak_Swipe_ArmSfx_Start(AnimationEventData& data) {
	}
	void GTS_Sneak_Swipe_ArmSfx_End(AnimationEventData& data) {
	}

	////////////////light

	void GTS_Sneak_Swipe_On_R(AnimationEventData& data) {
		//ManageCamera(&data.giant, true, 7.0);
		TriggerHandCollision_Right(&data.giant, Damage_Sneak_HandSwipe, 1.8, 1.0);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 4.0);
	}
	void GTS_Sneak_Swipe_On_L(AnimationEventData& data) {
		//ManageCamera(&data.giant, true, 4.0);
		TriggerHandCollision_Left(&data.giant, Damage_Sneak_HandSwipe, 1.8, 1.0);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 4.0);
	}
	void GTS_Sneak_Swipe_Off_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 4.0);
		//DisableCameraTracking(&data.giant);
		DisableHandCollisions(&data.giant);
	}
	void GTS_Sneak_Swipe_Off_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 4.0);
		//DisableCameraTracking(&data.giant);
		DisableHandCollisions(&data.giant);
	}

	///////////////strong

	void GTS_Sneak_Swipe_Power_On_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 10.0);
		TriggerHandCollision_Right(&data.giant, Damage_Sneak_HandSwipe_Strong, 1.4, 2.35);
		//ManageCamera(&data.giant, true, 4.0);
	}
	void GTS_Sneak_Swipe_Power_On_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 10.0);
		TriggerHandCollision_Left(&data.giant, Damage_Sneak_HandSwipe_Strong, 1.4, 2.35);
		//ManageCamera(&data.giant, true, 7.0);
	}
	void GTS_Sneak_Swipe_Power_Off_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 10.0);
		//DisableCameraTracking(&data.giant);
		DisableHandCollisions(&data.giant);
	}
	void GTS_Sneak_Swipe_Power_Off_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 10.0);
		//DisableCameraTracking(&data.giant);
		DisableHandCollisions(&data.giant);
	}
}

namespace Gts {
    
    void Animation_SneakSwipes::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_On_R", "Sneak", GTS_Sneak_Swipe_On_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_On_L", "Sneak", GTS_Sneak_Swipe_On_L);
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_Off_R", "Sneak", GTS_Sneak_Swipe_Off_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_Off_L", "Sneak", GTS_Sneak_Swipe_Off_L);


		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_Power_On_R", "Sneak", GTS_Sneak_Swipe_Power_On_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_Power_On_L", "Sneak", GTS_Sneak_Swipe_Power_On_L);
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_Power_Off_R", "Sneak", GTS_Sneak_Swipe_Power_Off_R);
		AnimationManager::RegisterEvent("GTS_Sneak_Swipe_Power_Off_L", "Sneak", GTS_Sneak_Swipe_Power_Off_L);
    }
}
