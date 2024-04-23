#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchObject.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Kicks.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/audio/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void PerformKick(std::string_view kick_type, float stamina_drain) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (!player->IsSneaking() && !player->AsActorState()->IsSprinting()) {
			float WasteStamina = stamina_drain * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim(kick_type, player);
			} else {
				TiredSound(player, "You're too tired for a kick");
			}
		}
	}

	void StartDamageAt(Actor* actor, float power, float crush, float pushpower, std::string_view node, DamageSource Source) {
		std::string name = std::format("LegKick_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();

		std::vector<ObjectRefHandle> Objects = GetNearbyObjects(actor);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Leg = find_node(giant, node);
			if (Leg) {
				DoDamageAtPoint_Cooldown(giant, Radius_Kick, power, Leg, 10, 0.30, crush, pushpower, Source);
				PushObjects(Objects, giant, Leg, pushpower, Radius_Kick, true);
			}
			return true;
		});
	}

	void StopAllDamageAndStamina(Actor* actor) {
		std::string name = std::format("LegKick_{}", actor->formID);
		DrainStamina(actor, "StaminaDrain_StrongKick", "DestructionBasics", false, 8.0);
		DrainStamina(actor, "StaminaDrain_Kick", "DestructionBasics", false, 4.0);
		TaskManager::Cancel(name);
	}

	void GTS_Kick_Camera_On_R(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::R_Foot);
	}
	void GTS_Kick_Camera_On_L(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::L_Foot);
	}
	void GTS_Kick_Camera_Off_R(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::R_Foot);
	}
	void GTS_Kick_Camera_Off_L(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::L_Foot);
	}

	void GTS_Kick_SwingLeg_L(AnimationEventData& data) {
	}
	void GTS_Kick_SwingLeg_R(AnimationEventData& data) {
	}

	void GTS_Kick_HitBox_On_R(AnimationEventData& data) {
		StartDamageAt(&data.giant, Damage_Kick, 1.8, Push_Kick_Normal, "NPC R Toe0 [RToe]", DamageSource::KickedRight);
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 4.0);
	}
	void GTS_Kick_HitBox_On_L(AnimationEventData& data) {
		StartDamageAt(&data.giant, Damage_Kick, 1.8, Push_Kick_Normal, "NPC L Toe0 [LToe]", DamageSource::KickedLeft);
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 4.0);
	}
	void GTS_Kick_HitBox_Off_R(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}
	void GTS_Kick_HitBox_Off_L(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}

	void GTS_Kick_HitBox_Power_On_R(AnimationEventData& data) {
		StartDamageAt(&data.giant, Damage_Kick_Strong, 1.8, Push_Kick_Strong, "NPC R Toe0 [RToe]", DamageSource::KickedRight);
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 8.0);
	}
	void GTS_Kick_HitBox_Power_On_L(AnimationEventData& data) {
		StartDamageAt(&data.giant, Damage_Kick_Strong, 1.8, Push_Kick_Strong, "NPC L Toe0 [LToe]", DamageSource::KickedLeft);
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 8.0);
	}
	void GTS_Kick_HitBox_Power_Off_R(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}
	void GTS_Kick_HitBox_Power_Off_L(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}

	// ======================================================================================
	//  Animation Triggers
	// ======================================================================================
	void LightKickLeftEvent(const InputEventData& data) {
		PerformKick("SwipeLight_Left", 35.0);
	}
	void LightKickRightEvent(const InputEventData& data) {
		PerformKick("SwipeLight_Right", 35.0);
	}

	void HeavyKickLeftEvent(const InputEventData& data) {
		PerformKick("SwipeHeavy_Left", 110.0);
	}
	void HeavyKickRightEvent(const InputEventData& data) {
		PerformKick("SwipeHeavy_Right", 110.0);
	}

	void HeavyKickRightLowEvent(const InputEventData& data) {
		PerformKick("StrongKick_Low_Right", 110.0);
	}
	void HeavyKickLeftLowEvent(const InputEventData& data) {
		PerformKick("StrongKick_Low_Left", 110.0);
	}
}

namespace Gts
{
	void AnimationKicks::RegisterEvents() {
		InputManager::RegisterInputEvent("LightKickLeft", LightKickLeftEvent);
		InputManager::RegisterInputEvent("LightKickRight", LightKickRightEvent);
		InputManager::RegisterInputEvent("HeavyKickLeft", HeavyKickLeftEvent);
		InputManager::RegisterInputEvent("HeavyKickRight", HeavyKickRightEvent);
		InputManager::RegisterInputEvent("HeavyKickRight_Low", HeavyKickRightLowEvent);
		InputManager::RegisterInputEvent("HeavyKickLeft_Low", HeavyKickLeftLowEvent);
		

		AnimationManager::RegisterEvent("GTS_Kick_Camera_On_R", "Kicks", GTS_Kick_Camera_On_R);
		AnimationManager::RegisterEvent("GTS_Kick_Camera_On_L", "Kicks", GTS_Kick_Camera_On_L);
		AnimationManager::RegisterEvent("GTS_Kick_Camera_Off_R", "Kicks", GTS_Kick_Camera_On_R);
		AnimationManager::RegisterEvent("GTS_Kick_Camera_Off_L", "Kicks", GTS_Kick_Camera_On_L);

		AnimationManager::RegisterEvent("GTS_Kick_SwingLeg_R", "Kicks", GTS_Kick_SwingLeg_R);
		AnimationManager::RegisterEvent("GTS_Kick_SwingLeg_L", "Kicks", GTS_Kick_SwingLeg_L);

		AnimationManager::RegisterEvent("GTS_Kick_HitBox_On_R", "Kicks", GTS_Kick_HitBox_On_R);
		AnimationManager::RegisterEvent("GTS_Kick_HitBox_Off_R", "Kicks", GTS_Kick_HitBox_Off_R);
		AnimationManager::RegisterEvent("GTS_Kick_HitBox_On_L", "Kicks", GTS_Kick_HitBox_On_L);
		AnimationManager::RegisterEvent("GTS_Kick_HitBox_Off_L", "Kicks", GTS_Kick_HitBox_Off_L);

		AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_On_R", "Kicks", GTS_Kick_HitBox_Power_On_R);
		AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_Off_R", "Kicks", GTS_Kick_HitBox_Power_Off_R);
		AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_On_L", "Kicks", GTS_Kick_HitBox_Power_On_L);
		AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_Off_L", "Kicks", GTS_Kick_HitBox_Power_Off_L);
	}

	void AnimationKicks::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StrongKick_Low_Left", "Kicks", "GTSBEH_HeavyKickLow_L");
		AnimationManager::RegisterTrigger("StrongKick_Low_Right", "Kicks", "GTSBEH_HeavyKickLow_R");
	}
}