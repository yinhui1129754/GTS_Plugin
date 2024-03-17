#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/StrongStomp.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
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
		"NPC L Foot [Lft ]",
		"NPC L Toe0 [LToe]",
		"NPC L Calf [LClf]",
		"NPC L PreRearCalf",
		"NPC L FrontThigh",
		"NPC L RearCalf [RrClf]",
	};
	const std::vector<std::string_view> L_LEG_RUMBLE_NODES = { 
		"NPC R Foot [Rft ]",
		"NPC R Toe0 [RToe]",
		"NPC R Calf [RClf]",
		"NPC R PreRearCalf",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
	};

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";


	void StartLegRumble(std::string_view tag, Actor& actor, float power, float halflife, std::string_view type) {
		if (type == "Left") {
			for (auto& node_name: L_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
			}
		} else if (type == "Right") {
			for (auto& node_name: R_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
			}
		}
	}

	void StopLegRumble(std::string_view tag, Actor& actor, std::string_view type) {
		if (type == "Left") {
			for (auto& node_name: L_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				GRumble::Stop(rumbleName, &actor);
			}
		} else if (type == "Right") {
			for (auto& node_name: R_LEG_RUMBLE_NODES) {
				std::string rumbleName = std::format("{}{}", tag, node_name);
				GRumble::Stop(rumbleName, &actor);
			}
		}
	}

	void DoImpactRumble(Actor* giant, float force, std::string_view node, std::string_view name) {
		if (HasSMT(giant)) {
			force *= 12.0;
		}
		GRumble::Once(name, giant, force, 0.05, node);
	}

	void DoSounds(Actor* giant, float animspeed, std::string_view feet) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = 8.0;
		}
		float scale = get_visual_scale(giant);
		Runtime::PlaySoundAtNode("HeavyStompSound", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlFootstepR", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlRumbleR", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
	}

	void StrongStomp_DoEverything(Actor* giant, float animSpeed, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float SMT = 1.0;
		float damage = 1.0;
		if (HasSMT(giant)) {
			SMT = 1.85; // Larger Dust
			damage = 1.25;
		}
		DoDamageEffect(giant, Damage_Stomp_Strong * damage * perk, Radius_Stomp_Strong, 5, 0.35, Event, 1.0, Source);
		DoImpactRumble(giant, SMT * animSpeed - (0.55 * 2), Node, rumble);
		DoDustExplosion(giant, 0.25 + SMT + (animSpeed * 0.05), Event, Node);

		DrainStamina(giant, "StaminaDrain_StrongStomp", "DestructionBasics", false, 3.4);

		DoFootstepSound(giant, SMT + (animSpeed/10), Event, Node);
		DoLaunch(giant, 1.05 * perk, 2.4 + animSpeed/2, Event);

		DoSounds(giant, 1.15 + animSpeed/20, Node);
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
		StartLegRumble("StrongStompR", data.giant, 0.35 *data.animSpeed - 0.35, 0.10, "Right");
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
		StartLegRumble("StrongStompL", data.giant, 0.35 *data.animSpeed - 0.35, 0.10, "Left");
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
		StopLegRumble("StrongStompR", data.giant, "Right");
	}
	void GTS_StrongStomp_LL_End(AnimationEventData& data) {
		StopLegRumble("StrongStompL", data.giant, "Left");
	}

	void GTS_StrongStomp_ImpactR(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StrongStomp_DoEverything(&data.giant, data.animSpeed, FootEvent::Right, DamageSource::CrushedRight, RNode, "HeavyStompR");
	}
	void GTS_StrongStomp_ImpactL(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StrongStomp_DoEverything(&data.giant, data.animSpeed, FootEvent::Left, DamageSource::CrushedLeft, LNode, "HeavyStompL");
	}

	void GTS_StrongStomp_ReturnRL_Start(AnimationEventData& data) {StartLegRumble("StrongStompR", data.giant, 0.25, 0.10, "Right");}
	void GTS_StrongStomp_ReturnLL_Start(AnimationEventData& data) {StartLegRumble("StrongStompL", data.giant, 0.25, 0.10, "Left");}

	void GTS_StrongStomp_ReturnRL_End(AnimationEventData& data) {
		StopLegRumble("StrongStompR", data.giant, "Right");
		ManageCamera(&data.giant, false, CameraTracking::R_Foot);
	}
	void GTS_StrongStomp_ReturnLL_End(AnimationEventData& data) {
		StopLegRumble("StrongStompL", data.giant, "Left");
		ManageCamera(&data.giant, false, CameraTracking::L_Foot);
	}
	void GTS_StrongStomp_End(AnimationEventData& data) {
		StopLegRumble("StrongStompR", data.giant, "Right");
		StopLegRumble("StrongStompL", data.giant, "Left");
	}


	void GTS_Next(AnimationEventData& data) {
		GRumble::Stop("StompR", &data.giant);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		auto giant = &data.giant;
		if (giant->formID == 0x14) {
			ResetCameraTracking();
		}

		GRumble::Stop("StompR", &data.giant);
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
