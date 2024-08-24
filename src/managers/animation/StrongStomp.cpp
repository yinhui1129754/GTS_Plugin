#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/StrongStomp.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/audio/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	const std::vector<std::string_view> R_LEG_RUMBLE_NODES = { 
		"NPC L Toe0 [LToe]",
		"NPC L Calf [LClf]",
		"NPC L PreRearCalf",
		"NPC L FrontThigh",
		"NPC L RearCalf [RrClf]",
	};
	const std::vector<std::string_view> L_LEG_RUMBLE_NODES = { 
		"NPC R Toe0 [RToe]",
		"NPC R Calf [RClf]",
		"NPC R PreRearCalf",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
	};

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";


	void StartLegRumbling(std::string_view tag, Actor& actor, float power, float halflife, bool right) {
		// Disabled because it messed up normal shake life-time, overriding it with the new one
		if (!right) {
			for (auto& node_name: L_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				Rumbling::Start(rumbleName, &actor, power,  halflife, node_name);
			}
		} else {
			for (auto& node_name: R_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				Rumbling::Start(rumbleName, &actor, power,  halflife, node_name);
			}
		}
	}

	void StopLegRumbling(std::string_view tag, Actor& actor, bool right) {
		if (!right) {
			for (auto& node_name: L_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				Rumbling::Stop(rumbleName, &actor);
			}
		} else {
			for (auto& node_name: R_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				Rumbling::Stop(rumbleName, &actor);
			}
		}
	}

	void DoImpactRumble(Actor* giant, std::string_view node, std::string_view name) {
		float shake_power = Rumble_Stomp_Strong;
		float smt = 1.0;
		if (HasSMT(giant)) {
			smt *= 1.5;
		}
		smt *= GetHighHeelsBonusDamage(giant, true);
		Rumbling::Once(name, giant, shake_power * smt, 0.0, node, 1.25);
	}

	void DoSounds(Actor* giant, float animspeed, std::string_view feet) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = 8.0;
		}
		float scale = get_visual_scale(giant);
		Runtime::PlaySoundAtNode("HeavyStompSound", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlFootstep", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlRumble", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
	}

	void StrongStomp_DoEverything(Actor* giant, float animSpeed, bool right, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float SMT = 1.0;
		float damage = 1.0;
		if (HasSMT(giant)) {
			SMT = 1.85; // Larger Dust
			damage = 1.25;
		}

		std::string taskname = std::format("StrongStompAttack_{}", giant->formID);
		ActorHandle giantHandle = giant->CreateRefHandle();

		float Start = Time::WorldTimeElapsed();
		
		TaskManager::RunFor(taskname, 1.0, [=](auto& update){ // Needed because anim has wrong timing
			if (!giantHandle) {
				return false;
			}

			float Finish = Time::WorldTimeElapsed();
			auto giant = giantHandle.get().get();

			if (Finish - Start > 0.07) { 

				DoDamageEffect(giant, Damage_Stomp_Strong * damage * perk, Radius_Stomp_Strong, 5, 0.35, Event, 1.0, Source);
				DoImpactRumble(giant, Node, rumble);
				DoDustExplosion(giant, 1.33 * (SMT + (animSpeed * 0.05)), Event, Node);

				DrainStamina(giant, "StaminaDrain_StrongStomp", "DestructionBasics", false, 3.4);

				DoFootstepSound(giant, SMT + (animSpeed/10), Event, Node);

				LaunchTask(giant, 1.05 * perk, 3.6 + animSpeed/2, Event);

				DoSounds(giant, 1.15 + animSpeed/20, Node);

				
				return false;
			}
			return true;
		});
		
	}

	///////////////////////////////////////////////////////////////////////
	//      EVENTS
	//////////////////////////////////////////////////////////////////////

	void GTS_StrongStomp_Start(AnimationEventData& data) {
		data.stage = 1;
		data.animSpeed = 1.35;
	}

	void GTS_StrongStomp_LR_Start(AnimationEventData& data) {
		auto giant = &data.giant;
		data.stage = 1;
		data.canEditAnimSpeed = true;
		if (data.giant.formID != 0x14) {
			data.animSpeed += GetRandomBoost()/3;
		}
		ManageCamera(giant, true, CameraTracking::R_Foot);
		DrainStamina(&data.giant, "StaminaDrain_StrongStomp", "DestructionBasics", true, 3.4);
	}

	void GTS_StrongStomp_LL_Start(AnimationEventData& data) {
		auto giant = &data.giant;
		data.stage = 1;
		data.canEditAnimSpeed = true;
		if (data.giant.formID != 0x14) {
			data.animSpeed += GetRandomBoost()/3;
		}
		ManageCamera(giant, true, CameraTracking::L_Foot);
		DrainStamina(&data.giant, "StaminaDrain_StrongStomp", "DestructionBasics", true, 3.4);
	}

	void GTS_StrongStomp_LR_Middle(AnimationEventData& data) {
		data.animSpeed = 1.55;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.55 + GetRandomBoost();
		}
	}
	void GTS_StrongStomp_LL_Middle(AnimationEventData& data) {
		data.animSpeed = 1.55;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.55 + GetRandomBoost();
		}
	}
	void GTS_StrongStomp_LR_End(AnimationEventData& data) {
		StopLegRumbling("StrongStompR", data.giant, true);
	}
	void GTS_StrongStomp_LL_End(AnimationEventData& data) {
		StopLegRumbling("StrongStompL", data.giant, false);
	}

	void GTS_StrongStomp_ImpactR(AnimationEventData& data) {
		float SavedSpeed = data.animSpeed;

		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StrongStomp_DoEverything(&data.giant, SavedSpeed, true, FootEvent::Right, DamageSource::CrushedRight, RNode, "HeavyStompR");
	}
	void GTS_StrongStomp_ImpactL(AnimationEventData& data) {
		float SavedSpeed = data.animSpeed;

		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StrongStomp_DoEverything(&data.giant, SavedSpeed, false, FootEvent::Left, DamageSource::CrushedLeft, LNode, "HeavyStompL");
	}

	void GTS_StrongStomp_ReturnRL_Start(AnimationEventData& data) {StartLegRumbling("StrongStompR", data.giant, 0.25, 0.10, true);}
	void GTS_StrongStomp_ReturnLL_Start(AnimationEventData& data) {StartLegRumbling("StrongStompL", data.giant, 0.25, 0.10, false);}

	void GTS_StrongStomp_ReturnRL_End(AnimationEventData& data) {
		StopLegRumbling("StrongStompR", data.giant, true);
		ManageCamera(&data.giant, false, CameraTracking::R_Foot);
	}
	void GTS_StrongStomp_ReturnLL_End(AnimationEventData& data) {
		StopLegRumbling("StrongStompL", data.giant, false);
		ManageCamera(&data.giant, false, CameraTracking::L_Foot);
	}
	void GTS_StrongStomp_End(AnimationEventData& data) {
		StopLegRumbling("StrongStompR", data.giant, true);
		StopLegRumbling("StrongStompL", data.giant, false);
	}


	void GTS_Next(AnimationEventData& data) {
		Rumbling::Stop("StompR", &data.giant);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		auto giant = &data.giant;
		if (giant->formID == 0x14) {
			ResetCameraTracking();
		}

		Rumbling::Stop("StompR", &data.giant);
	}

	void RightStrongStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		float WasteStamina = 70.0 * GetWasteMult(player);
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("StrongStompRight", player);
		} else {
			TiredSound(player, "You're too tired to perform heavy stomp");
		}
	}

	void LeftStrongStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		float WasteStamina = 70.0 * GetWasteMult(player);
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			//BlockFirstPerson(player, true);
			AnimationManager::StartAnim("StrongStompLeft", player);
		} else {
			TiredSound(player, "You're too tired to perform heavy stomp");
		}
	}
}

namespace Gts
{
	void AnimationStrongStomp::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_StrongStomp_Start", "StrongStomp", GTS_StrongStomp_Start);
		AnimationManager::RegisterEvent("GTS_StrongStomp_LR_Start", "StrongStomp", GTS_StrongStomp_LR_Start);
		AnimationManager::RegisterEvent("GTS_StrongStomp_LL_Start", "StrongStomp", GTS_StrongStomp_LL_Start);
		AnimationManager::RegisterEvent("GTS_StrongStomp_LR_Middle", "StrongStomp", GTS_StrongStomp_LR_Middle);
		AnimationManager::RegisterEvent("GTS_StrongStomp_LL_Middle", "StrongStomp", GTS_StrongStomp_LL_Middle);
		AnimationManager::RegisterEvent("GTS_StrongStomp_LR_End", "StrongStomp", GTS_StrongStomp_LR_End);
		AnimationManager::RegisterEvent("GTS_StrongStomp_LL_End", "StrongStomp", GTS_StrongStomp_LL_End);
		AnimationManager::RegisterEvent("GTS_StrongStomp_ImpactR", "StrongStomp", GTS_StrongStomp_ImpactR);
		AnimationManager::RegisterEvent("GTS_StrongStomp_ImpactL", "StrongStomp", GTS_StrongStomp_ImpactL);
		AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnRL_Start", "StrongStomp", GTS_StrongStomp_ReturnRL_Start);
		AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnLL_Start", "StrongStomp", GTS_StrongStomp_ReturnLL_Start);
		AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnRL_End", "StrongStomp", GTS_StrongStomp_ReturnRL_End);
		AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnLL_End", "StrongStomp", GTS_StrongStomp_ReturnLL_End);
		AnimationManager::RegisterEvent("GTS_StrongStomp_End", "StrongStomp", GTS_StrongStomp_End);
		AnimationManager::RegisterEvent("GTS_Next", "StrongStomp", GTS_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "StrongStomp", GTSBEH_Exit);

		InputManager::RegisterInputEvent("RightStomp_Strong", RightStrongStompEvent);
		InputManager::RegisterInputEvent("LeftStomp_Strong", LeftStrongStompEvent);
	}

	void AnimationStrongStomp::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StrongStompRight", "Stomp", "GTSBeh_StrongStomp_StartRight");
		AnimationManager::RegisterTrigger("StrongStompLeft", "Stomp", "GTSBeh_StrongStomp_StartLeft");
	}
}
