#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Crawling.hpp"
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

	void EnableHandTracking(Actor* giant, CrawlEvent kind, bool enable) {
		if (AllowCameraTracking() && giant->formID == 0x14) {
			auto& sizemanager = SizeManager::GetSingleton();
			if (kind == CrawlEvent::RightHand) {
				sizemanager.SetTrackedBone(giant, enable, CameraTracking::Hand_Right);
			} else if (kind == CrawlEvent::LeftHand) {
				sizemanager.SetTrackedBone(giant, enable, CameraTracking::Hand_Left);
			}
		}
	}

	void GTS_Crawl_Knee_Trans_Impact(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.80, Damage_Crawl_KneeImpact_Drop, CrawlEvent::LeftKnee, "LeftKnee", 0.78, Radius_Crawl_KneeImpact_Fall, 1.15, DamageSource::KneeDropLeft);
		DoCrawlingFunctions(giant, scale, 1.80, Damage_Crawl_KneeImpact_Drop, CrawlEvent::RightKnee, "RightKnee", 0.78, Radius_Crawl_KneeImpact_Fall, 1.15, DamageSource::KneeDropRight);
		//                                     launch power                                ^    ^ --- Size Damage Radius
		//                                                                             Launch       ^ -- crush threshold
		//                                                                             Radius
	}

	void GTS_Crawl_Hand_Trans_Impact(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.60, Damage_Crawl_HandImpact_Drop, CrawlEvent::LeftHand, "LeftHand", 0.70, Radius_Crawl_HandImpact_Fall, 1.15, DamageSource::HandDropLeft);
		DoCrawlingFunctions(giant, scale, 1.60, Damage_Crawl_HandImpact_Drop, CrawlEvent::RightHand, "RightHand", 0.70, Radius_Crawl_HandImpact_Fall, 1.15, DamageSource::HandDropRight);
	}

	void GTSCrawl_KneeImpact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, Damage_Crawl_KneeImpact, CrawlEvent::LeftKnee, "LeftKnee", 0.60, Radius_Crawl_KneeImpact, 1.25, DamageSource::KneeLeft);
	}
	void GTSCrawl_KneeImpact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, Damage_Crawl_KneeImpact, CrawlEvent::RightKnee, "RightKnee", 0.6, Radius_Crawl_KneeImpact, 1.25, DamageSource::KneeRight);
	}
	void GTSCrawl_HandImpact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		if (IsTransferingTiny(giant)) {
			return; // Prevent effects from left hand
		}
		DoCrawlingFunctions(giant, scale, 1.10, Damage_Crawl_HandImpact, CrawlEvent::LeftHand, "LeftHand", 0.55, Radius_Crawl_HandImpact, 1.25, DamageSource::HandCrawlLeft);
	}
	void GTSCrawl_HandImpact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.10, Damage_Crawl_HandImpact, CrawlEvent::RightHand, "RightHand", 0.55, Radius_Crawl_HandImpact, 1.25, DamageSource::HandCrawlRight);
		//                                                                               ^    ^ --- Size Damage Radius
		//                                                                             Launch
		//                                                                             Radius
	}

	void GTSCrawl_Slam_Raise_Arm_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", true, 1.4);
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}

	void GTSCrawl_Slam_Raise_Arm_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", true, 1.4);
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}
	void GTSCrawl_SlamStrong_Raise_Arm_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", true, 2.3);
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}

	void GTSCrawl_SlamStrong_Raise_Arm_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", true, 2.3);
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}

	void GTSCrawl_Slam_Lower_Arm_R(AnimationEventData& data) {
	}
	void GTSCrawl_SlamStrong_Lower_Arm_R(AnimationEventData& data) {
	}
	void GTSCrawl_Slam_Lower_Arm_L(AnimationEventData& data) {
	}
	void GTSCrawl_SlamStrong_Lower_Arm_L(AnimationEventData& data) {
	}

	void GTSCrawl_Slam_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, Damage_Crawl_HandSlam, CrawlEvent::RightHand, "RightHandRumble", 0.60, Radius_Crawl_Slam, 1.15, DamageSource::HandSlamRight);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}
	void GTSCrawl_Slam_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, Damage_Crawl_HandSlam, CrawlEvent::LeftHand, "LeftHandRumble", 0.60, Radius_Crawl_Slam, 1.15, DamageSource::HandSlamLeft);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}

	void GTSCrawl_SlamStrong_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, Damage_Crawl_HandSlam_Strong, CrawlEvent::RightHand, "RightHandRumble", 0.80, Radius_Crawl_Slam_Strong, 1.0, DamageSource::HandSlamRight);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}
	void GTSCrawl_SlamStrong_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, Damage_Crawl_HandSlam_Strong, CrawlEvent::LeftHand, "RightHandRumble", 0.80, Radius_Crawl_Slam_Strong, 1.0, DamageSource::HandSlamLeft);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}

	void GTSCrawl_Slam_Cam_Off_R(AnimationEventData& data) {
		auto giant = &data.giant;
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, false);
	}
	void GTSCrawl_Slam_Cam_Off_L(AnimationEventData& data) {
		auto giant = &data.giant;
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, false);
	}

	/////////////////////////////////////////////////////////Swipe Attacks//////////////////////////////////////////

	void TriggerHandCollision_Right(Actor* actor, float power, float crush, float pushpower) {
		std::string name = std::format("HandCollide_R_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC R Forearm [RLar]");
			auto Arm = find_node(giant, "NPC R Hand [RHnd]");
			if (Uarm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, Damage_Crawl_HandSwipe * power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeRight);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, Damage_Crawl_HandSwipe * power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeRight);
			}
			return true;
		});
	}

	void TriggerHandCollision_Left(Actor* actor, float power, float crush, float pushpower) {
		std::string name = std::format("HandCollide_L_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC L Forearm [LLar]");
			auto Arm = find_node(giant, "NPC L Hand [LHnd]");
			if (Uarm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, Damage_Crawl_HandSwipe * power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeLeft);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, Radius_Sneak_HandSwipe, Damage_Crawl_HandSwipe * power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeLeft);
			}
			return true;
		});
	}
	void DisableHandCollisions(Actor* actor) {
		std::string name = std::format("HandCollide_L_{}", actor->formID);
		std::string name2 = std::format("HandCollide_R_{}", actor->formID);
		TaskManager::Cancel(name);
		TaskManager::Cancel(name2);
	}

	void DisableHandTrackingTask(Actor* giant) { // Used to disable camera with some delay
		std::string name = std::format("CameraOff_{}", giant->formID);
		auto gianthandle = giant->CreateRefHandle();
		auto FrameA = Time::FramesElapsed();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 60.0/GetAnimationSlowdown(giantref)) {
				return true;
			}

			EnableHandTracking(giantref, CrawlEvent::RightHand, false);
			EnableHandTracking(giantref, CrawlEvent::LeftHand, false);

			return false;
		});
	}

	void GTS_Crawl_Swipe_ArmSfx_Start(AnimationEventData& data) {
	}
	void GTS_Crawl_Swipe_ArmSfx_End(AnimationEventData& data) {
	}

	void GTS_Crawl_Swipe_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 1.0, 1.6, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 4.0);
	}
	void GTS_Crawl_Swipe_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 1.0, 1.6, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 4.0);
	}
	void GTS_Crawl_Swipe_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 4.0);
	}
	void GTS_Crawl_Swipe_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 4.0);
	}

	void GTS_Crawl_Swipe_Power_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 2.0, 1.3, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 10.0);
	}
	void GTS_Crawl_Swipe_Power_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 2.0, 1.3, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 10.0);
	}
	void GTS_Crawl_Swipe_Power_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 10.0);
	}
	void GTS_Crawl_Swipe_Power_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 10.0);
	}

	void LightSwipeLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (player->IsSneaking()) {
			float WasteStamina = 25.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				Utils_UpdateHighHeelBlend(player, false);
				AnimationManager::StartAnim("SwipeLight_Left", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}
	void LightSwipeRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (player->IsSneaking()) {
			float WasteStamina = 25.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				Utils_UpdateHighHeelBlend(player, false);
				AnimationManager::StartAnim("SwipeLight_Right", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}

	void HeavySwipeLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (player->IsSneaking()) {
			float WasteStamina = 70.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				Utils_UpdateHighHeelBlend(player, false);
				AnimationManager::StartAnim("SwipeHeavy_Left", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}
	void HeavySwipeRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (player->IsSneaking()) {
			float WasteStamina = 70.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				Utils_UpdateHighHeelBlend(player, false);
				AnimationManager::StartAnim("SwipeHeavy_Right", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}
}

namespace Gts
{
	void AnimationCrawling::RegisterEvents() {

		InputManager::RegisterInputEvent("LightSwipeLeft", LightSwipeLeftEvent);
		InputManager::RegisterInputEvent("LightSwipeRight", LightSwipeRightEvent);
		InputManager::RegisterInputEvent("HeavySwipeLeft", HeavySwipeLeftEvent);
		InputManager::RegisterInputEvent("HeavySwipeRight", HeavySwipeRightEvent);

		AnimationManager::RegisterEvent("GTS_Crawl_Knee_Trans_Impact", "Crawl", GTS_Crawl_Knee_Trans_Impact);
		AnimationManager::RegisterEvent("GTS_Crawl_Hand_Trans_Impact", "Crawl", GTS_Crawl_Hand_Trans_Impact);

		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_L", "Crawl", GTSCrawl_KneeImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_R", "Crawl", GTSCrawl_KneeImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_L", "Crawl", GTSCrawl_HandImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_R", "Crawl", GTSCrawl_HandImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Raise_Arm_R", "Crawl", GTSCrawl_Slam_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Raise_Arm_L", "Crawl", GTSCrawl_Slam_Raise_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Raise_Arm_R", "Crawl", GTSCrawl_SlamStrong_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Raise_Arm_L", "Crawl", GTSCrawl_SlamStrong_Raise_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Lower_Arm_R", "Crawl", GTSCrawl_Slam_Lower_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Lower_Arm_L", "Crawl", GTSCrawl_Slam_Lower_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Lower_Arm_R", "Crawl", GTSCrawl_SlamStrong_Lower_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Lower_Arm_L", "Crawl", GTSCrawl_SlamStrong_Lower_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Impact_R", "Crawl", GTSCrawl_Slam_Impact_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Impact_L", "Crawl", GTSCrawl_Slam_Impact_L);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Impact_R", "Crawl", GTSCrawl_SlamStrong_Impact_R);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Impact_L", "Crawl", GTSCrawl_SlamStrong_Impact_L);

		AnimationManager::RegisterEvent("GTSCrawl_Slam_Cam_Off_L", "Crawl", GTSCrawl_Slam_Cam_Off_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Cam_Off_R", "Crawl", GTSCrawl_Slam_Cam_Off_R);

		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_On_R", "Crawl", GTS_Crawl_Swipe_On_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_On_L", "Crawl", GTS_Crawl_Swipe_On_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Off_R", "Crawl", GTS_Crawl_Swipe_Off_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Off_L", "Crawl", GTS_Crawl_Swipe_Off_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_On_R", "Crawl", GTS_Crawl_Swipe_Power_On_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_On_L", "Crawl", GTS_Crawl_Swipe_Power_On_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_Off_R", "Crawl", GTS_Crawl_Swipe_Power_Off_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_Off_L", "Crawl", GTS_Crawl_Swipe_Power_Off_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_ArmSfx_Start", "Crawl", GTS_Crawl_Swipe_ArmSfx_Start);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_ArmSfx_End", "Crawl", GTS_Crawl_Swipe_ArmSfx_End);
	}

	void AnimationCrawling::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SwipeLight_Left", "Crawl", "GTSBeh_SwipeLight_L");
		AnimationManager::RegisterTrigger("SwipeLight_Right", "Crawl", "GTSBeh_SwipeLight_R");
		AnimationManager::RegisterTrigger("SwipeHeavy_Right", "Crawl", "GTSBeh_SwipeHeavy_R");
		AnimationManager::RegisterTrigger("SwipeHeavy_Left", "Crawl", "GTSBeh_SwipeHeavy_L");
		AnimationManager::RegisterTrigger("CrawlON", "Crawl", "GTSBeh_Crawl_On");
		AnimationManager::RegisterTrigger("CrawlOFF", "Crawl", "GTSBeh_Crawl_Off");
	}
}