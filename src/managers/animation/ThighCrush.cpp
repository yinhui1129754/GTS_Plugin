// Animation: ThighCrush
//  - Stages
//    - "GTStosit",                     // [0] Start air rumble and camera shake
//    - "GTSsitloopenter",              // [1] Sit down completed
//    - "GTSsitloopstart",              // [2] enter sit crush loop
//    - "GTSsitloopend",                // [3] unused
//    - "GTSsitcrushlight_start",       // [4] Start Spreading legs
//    - "GTSsitcrushlight_end",         // [5] Legs fully spread
//    - "GTSsitcrushheavy_start",       // [6] Start Closing legs together
//    - "GTSsitcrushheavy_end",         // [7] Legs fully closed
//    - "GTSsitloopexit",               // [8] stand up, small air rumble and camera shake
//    - "GTSstandR",                    // [9] feet collides with ground when standing up
//    - "GTSstandL",                    // [10]
//    - "GTSstandRS",                   // [11] Silent impact of right feet
//    - "GTStoexit",                    // [12] Leave animation, disable air rumble and such

#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/ThighCrush.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	const std::vector<std::string_view> BODY_RUMBLE_NODES = { // used for body rumble
		"NPC COM [COM ]",
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	const std::vector<std::string_view> LEG_RUMBLE_NODES = { // used with Anim_ThighCrush
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	void LegRumbleOnce(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Once(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartLegRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartBodyRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: BODY_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopLegRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}

	void RunThighCollisionTask(Actor* giant, bool right, bool CooldownCheck, float radius, float damage, float bbmult, float crush_threshold, int random, std::string_view tn) {
		std::string name = std::format("ThighCrush_{}_{}", giant->formID, tn);
		auto gianthandle = giant->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			if (!IsThighCrushing(giantref)) {
				return false; //Disable it once we leave Thigh Crush state
			}
			
			ApplyThighDamage(giant, right, CooldownCheck, radius, damage, bbmult, crush_threshold, random, DamageSource::ThighCrushed);

			return true; // Cancel it
		});
	}

	void RunButtCollisionTask(Actor* giant) {
		std::string name = std::format("ButtCrush_{}", giant->formID);
		auto gianthandle = giant->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto ThighL = find_node(giantref, "NPC L Thigh [LThg]");
			auto ThighR = find_node(giantref, "NPC R Thigh [RThg]");

			if (!IsThighCrushing(giantref)) {
				return false; //Disable it once we leave Thigh Crush state
			}
			if (ThighL && ThighR) {
				DoDamageAtPoint(giantref, Radius_ThighCrush_Butt_DOT, Damage_ThighCrush_Butt_DOT * TimeScale(), ThighL, 100, 0.20, 2.5, DamageSource::Booty);
				DoDamageAtPoint(giantref, Radius_ThighCrush_Butt_DOT, Damage_ThighCrush_Butt_DOT * TimeScale(), ThighR, 100, 0.20, 2.5, DamageSource::Booty);
				return true;
			}
			return false; // Cancel it if we don't have these bones
		});
	}

	void ThighCrush_GetUpDamage(Actor* giant, float animSpeed, float mult, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float scale = get_visual_scale(giant);
		float speed = animSpeed;
		float volume = scale * 0.10 * speed;
		float perk = GetPerkBonus_Thighs(giant);

		DoDamageEffect(giant, Damage_ThighCrush_Stand_Up * mult * perk, Radius_ThighCrush_Stand_Up, 25, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoLaunch(giant, 0.65 * mult * perk, 1.55 * animSpeed, FootEvent::Right);
		GRumble::Once(rumble, giant, volume * 4, 0.10, RNode);
		DoFootstepSound(giant, 1.05, FootEvent::Right, RNode);
		DoDustExplosion(giant, 1.1, FootEvent::Right, RNode);
	}

	////////////////////////////////////////////////////////////////////////////////////
	//// EVENTS
	///////////////////////////////////////////////////////////////////////////////////

	void GTStosit(AnimationEventData& data) {
		float speed = data.animSpeed;
		StartLegRumble("ThighCrush", data.giant, 0.10, 0.10);
		ManageCamera(&data.giant, true, CameraTracking::Thigh_Crush); // Track feet

		RunThighCollisionTask(&data.giant, true, false, Radius_ThighCrush_Idle, Damage_ThighCrush_Legs_Idle, 0.02, 2.0, 600, "ThighIdle_R");
		RunThighCollisionTask(&data.giant, false, false, Radius_ThighCrush_Idle, Damage_ThighCrush_Legs_Idle, 0.02, 2.0, 600, "ThighIdle_L");

		RunButtCollisionTask(&data.giant);
		data.stage = 1;
	}

	void GTSsitloopenter(AnimationEventData& data) {
		float speed = data.animSpeed;
		StartLegRumble("ThighCrush", data.giant, 0.12 * speed, 0.10);
		data.disableHH = true;
		data.HHspeed = 4.0;
		data.stage = 2;
	}

	void GTSsitloopstart(AnimationEventData& data) {
		StopLegRumble("ThighCrush", data.giant);
		data.currentTrigger = 1;
		data.stage = 3;
	}

	void GTSsitloopend(AnimationEventData& data) {
		data.stage = 4;
	}

	void GTSsitcrushlight_start(AnimationEventData& data) {
		auto giant = &data.giant;
		StartLegRumble("ThighCrush", data.giant, 0.18, 0.12);
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", true, 1.0); // < Start Light Stamina Drain


		std::string name_l = std::format("ThighCrush_{}_ThighIdle_R", giant->formID);
		std::string name_r = std::format("ThighCrush_{}_ThighIdle_L", giant->formID);
		TaskManager::Cancel(name_l);
		TaskManager::Cancel(name_r);

		RunThighCollisionTask(&data.giant, true, true, Radius_ThighCrush_Spread_Out, Damage_ThighCrush_CrossLegs_Out, 0.10, 1.70, 50, "ThighLight_R");
		RunThighCollisionTask(&data.giant, false, true, Radius_ThighCrush_Spread_Out, Damage_ThighCrush_CrossLegs_Out, 0.10, 1.70, 50, "ThighLight_L");

		data.stage = 5;
	}

	void GTSsitcrushlight_end(AnimationEventData& data) {
		auto giant = &data.giant;
		data.currentTrigger = 2;
		data.canEditAnimSpeed = true;
		LegRumbleOnce("ThighCrush_End", data.giant, 0.22, 0.20);
		StopLegRumble("ThighCrush", data.giant);
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", false, 1.0); // < Stop Light Stamina Drain

		std::string name_l = std::format("ThighCrush_{}_ThighLight_R", giant->formID);
		std::string name_r = std::format("ThighCrush_{}_ThighLight_L", giant->formID);
		TaskManager::Cancel(name_l);
		TaskManager::Cancel(name_r);

		data.stage = 6;
	}

	void GTSsitcrushheavy_start(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", true, 2.5); // < - Start HEAVY Stamina Drain

		std::string name_l = std::format("ThighCrush_{}_ThighIdle_R", giant->formID);
		std::string name_r = std::format("ThighCrush_{}_ThighIdle_L", giant->formID);
		TaskManager::Cancel(name_l);
		TaskManager::Cancel(name_r);

		RunThighCollisionTask(&data.giant, true, true, Radius_ThighCrush_Spread_In, Damage_ThighCrush_CrossLegs_In, 0.25, 1.4, 25, "ThighHeavy_R");
		RunThighCollisionTask(&data.giant, false, true, Radius_ThighCrush_Spread_In, Damage_ThighCrush_CrossLegs_In, 0.25, 1.4, 25, "ThighHeavy_L");

		StartLegRumble("ThighCrushHeavy", data.giant, 0.35, 0.10);
		data.stage = 5;
	}

	void GTSsitcrushheavy_end(AnimationEventData& data) {
		auto giant = &data.giant;
		data.currentTrigger = 2;
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", false, 2.5); // < Stop Heavy Stamina Drain
		LegRumbleOnce("ThighCrushHeavy_End", data.giant, 0.50, 0.15);
		StopLegRumble("ThighCrushHeavy", data.giant);

		std::string name_l = std::format("ThighCrush_{}_ThighHeavy_R", giant->formID);
		std::string name_r = std::format("ThighCrush_{}_ThighHeavy_L", giant->formID);
		TaskManager::Cancel(name_l);
		TaskManager::Cancel(name_r);

		RunThighCollisionTask(&data.giant, true, false, Radius_ThighCrush_Idle, Damage_ThighCrush_Legs_Idle, 0.02, 3.0, 600, "ThighIdle_R");
		RunThighCollisionTask(&data.giant, false, false, Radius_ThighCrush_Idle, Damage_ThighCrush_Legs_Idle, 0.02, 3.0, 600, "ThighIdle_L");

		data.stage = 6;
	}

	void GTSsitloopexit(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;

		data.HHspeed = 1.0;
		data.disableHH = false;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StartBodyRumble("BodyRumble", data.giant, 0.25, 0.12);
		data.stage = 8;
	}

	void GTSstandR(AnimationEventData& data) {
		// do stand up damage
		ThighCrush_GetUpDamage(&data.giant, data.animSpeed, 1.0, FootEvent::Right, DamageSource::CrushedRight, RNode, "ThighCrushStompR");
		data.stage = 9;
	}

	void GTSstandL(AnimationEventData& data) {
		// do stand up damage
		ThighCrush_GetUpDamage(&data.giant, data.animSpeed, 1.0, FootEvent::Left, DamageSource::CrushedLeft, LNode, "ThighCrushStompL");
		data.stage = 9;
	}

	void GTSstandRS(AnimationEventData& data) {
		// do weaker stand up damage
		ThighCrush_GetUpDamage(&data.giant, data.animSpeed, 0.8, FootEvent::Right, DamageSource::CrushedRight, RNode, "ThighCrushStompR_S");
		data.stage = 9;
	}
	void GTSBEH_Next(AnimationEventData& data) {
		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
	}
	void GTStoexit(AnimationEventData& data) {
		// Going to exit
		StopLegRumble("BodyRumble", data.giant);
		ManageCamera(&data.giant, false, CameraTracking::Thigh_Crush); // Un-track feet
	}
	void GTSBEH_Exit(AnimationEventData& data) {
		// Final exit
		data.stage = 0;
	}

	void ThighCrushEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 2)) {
			return;
		}
		AnimationManager::StartAnim("ThighLoopEnter", player);
	}

	void ThighCrushKillEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (IsGtsBusy(player)) {
			float WasteStamina = 40.0;
			if (Runtime::HasPerk(player, "KillerThighs")) {
				WasteStamina *= 0.65;
			}
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("ThighLoopAttack", player);
			} else {
				if (IsThighCrushing(player)) {
					TiredSound(player, "You're too tired to perform thighs attack");
				}
			}
		}
	}

	void ThighCrushSpareEvent(const InputEventData& data) {
		if (!IsFreeCameraEnabled()) {
			auto player = PlayerCharacter::GetSingleton();
			if (IsGtsBusy(player)) {
				AnimationManager::StartAnim("ThighLoopExit", player);
			}
		}
	}
}

namespace Gts
{
	void AnimationThighCrush::RegisterEvents() {
		AnimationManager::RegisterEvent("GTStosit", "ThighCrush", GTStosit);
		AnimationManager::RegisterEvent("GTSsitloopenter", "ThighCrush", GTSsitloopenter);
		AnimationManager::RegisterEvent("GTSsitloopstart", "ThighCrush", GTSsitloopstart);
		AnimationManager::RegisterEvent("GTSsitloopend", "ThighCrush", GTSsitloopend);
		AnimationManager::RegisterEvent("GTSsitcrushlight_start", "ThighCrush", GTSsitcrushlight_start);
		AnimationManager::RegisterEvent("GTSsitcrushlight_end", "ThighCrush", GTSsitcrushlight_end);
		AnimationManager::RegisterEvent("GTSsitcrushheavy_start", "ThighCrush", GTSsitcrushheavy_start);
		AnimationManager::RegisterEvent("GTSsitcrushheavy_end", "ThighCrush", GTSsitcrushheavy_end);
		AnimationManager::RegisterEvent("GTSsitloopexit", "ThighCrush", GTSsitloopexit);
		AnimationManager::RegisterEvent("GTSstandR", "ThighCrush", GTSstandR);
		AnimationManager::RegisterEvent("GTSstandL", "ThighCrush", GTSstandL);
		AnimationManager::RegisterEvent("GTSstandRS", "ThighCrush", GTSstandRS);
		AnimationManager::RegisterEvent("GTStoexit", "ThighCrush", GTStoexit);
		AnimationManager::RegisterEvent("GTSBEH_Next", "ThighCrush", GTSBEH_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "ThighCrush", GTSBEH_Exit);

		InputManager::RegisterInputEvent("ThighCrush", ThighCrushEvent);
		InputManager::RegisterInputEvent("ThighCrushKill", ThighCrushKillEvent);
		InputManager::RegisterInputEvent("ThighCrushSpare", ThighCrushSpareEvent);
	}

	void AnimationThighCrush::RegisterTriggers() {
		AnimationManager::RegisterTriggerWithStages("ThighCrush", "ThighCrush", {"GTSBeh_TriggerSitdown", "GTSBeh_StartThighCrush", "GTSBeh_LeaveSitdown"});
		AnimationManager::RegisterTrigger("ThighLoopEnter", "ThighCrush", "GTSBeh_TriggerSitdown");
		AnimationManager::RegisterTrigger("ThighLoopAttack", "ThighCrush", "GTSBeh_StartThighCrush");
		AnimationManager::RegisterTrigger("ThighLoopExit", "ThighCrush", "GTSBeh_LeaveSitdown");
		AnimationManager::RegisterTrigger("ThighLoopFull", "ThighCrush", "GTSBeh_ThighAnimationFull");
	}
}
