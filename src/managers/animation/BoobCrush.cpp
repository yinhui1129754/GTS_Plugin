#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/BoobCrush.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

/*
   GTS_BoobCrush_Smile_On
   GTS_BoobCrush_Smile_Off
   GTS_BoobCrush_TrackBody        (Enables camera tracking)
   GTS_BoobCrush_UnTrackBody  (Disables it)
   GTS_BoobCrush_BreastImpact  (Damage everything under breasts, on impact)
   GTS_BoobCrush_DOT_Start       (When we want to deal damage over time when we do long idle with swaying legs)
   GTS_BoobCrush_DOT_End
   GTS_BoobCrush_Grow_Start
   GTS_BoobCrush_Grow_Stop


 */

namespace {

	const std::vector<std::string_view> ALL_RUMBLE_NODES = { // used for body rumble
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
		"NPC L UpperarmTwist1 [LUt1]",
		"NPC L UpperarmTwist2 [LUt2]",
		"NPC L Forearm [LLar]",
		"NPC L ForearmTwist2 [LLt2]",
		"NPC L ForearmTwist1 [LLt1]",
		"NPC L Hand [LHnd]",
		"NPC R UpperarmTwist1 [RUt1]",
		"NPC R UpperarmTwist2 [RUt2]",
		"NPC R Forearm [RLar]",
		"NPC R ForearmTwist2 [RLt2]",
		"NPC R ForearmTwist1 [RLt1]",
		"NPC R Hand [RHnd]",
		"NPC L Breast",
		"NPC R Breast",
		"L Breast03",
		"R Breast03",
	};

	const std::vector<std::string_view> BODY_NODES = {
		"NPC R Thigh [RThg]",
		"NPC L Thigh [LThg]",
		"NPC R Butt",
		"NPC L Butt",
		"NPC Spine [Spn0]",
		"NPC Spine1 [Spn1]",
		"NPC Spine2 [Spn2]",
	};

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void StartRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: ALL_RUMBLE_NODES) {
			std::string rumbleName = std::format("BoobCrush_{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power, halflife, node_name);
		}
	}

	void StopRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: ALL_RUMBLE_NODES) {
			std::string rumbleName = std::format("BoobCrush_{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}

	float GetBoobCrushDamage(Actor* actor) {
		float damage = 1.0;
		if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
			damage += 0.30;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
			damage += 0.70;
		}
		return damage;
	}

	void ModGrowthCount(Actor* giant, float value, bool reset) {
		auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			transient->ButtCrushGrowthAmount += value;
			if (reset) {
				transient->ButtCrushGrowthAmount = 0.0;
			}
		}
	}

	float GetGrowthCount(Actor* giant) {
		auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			return transient->ButtCrushGrowthAmount;
		}
		return 1.0;
	}

	void SetBonusSize(Actor* giant, float value, bool reset) {
		auto saved_data = Persistent::GetSingleton().GetData(giant);
		if (saved_data) {
			saved_data->bonus_max_size += value;
			if (reset) {
				update_target_scale(giant, -saved_data->bonus_max_size, SizeEffectType::kNeutral);
				if (get_target_scale(giant) < get_natural_scale(giant)) {
					set_target_scale(giant, get_natural_scale(giant)); // Protect against going into negatives
				}
				saved_data->bonus_max_size = 0;
			}
		}
	}

	void StartDamageOverTime(Actor* giant) {
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("BreastDOT_{}", giant->formID);
		float damage = GetBoobCrushDamage(giant) * TimeScale();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			auto BreastL = find_node(giantref, "NPC L Breast");
			auto BreastR = find_node(giantref, "NPC R Breast");
			auto BreastL03 = find_node(giantref, "L Breast03");
			auto BreastR03 = find_node(giantref, "R Breast03");

			if (!IsButtCrushing(giantref)) {
				return false;
			}

			for (auto Nodes: BODY_NODES) {
				auto Node = find_node(giantref, Nodes);
				if (Node) {
					std::string rumbleName = std::format("Node: {}", Nodes);
					DoDamageAtPoint(giant, Radius_BreastCrush_BodyDOT, Damage_BreastCrush_BodyDOT * damage, Node, 400, 0.10, 1.33, DamageSource::BodyCrush);
					GRumble::Once(rumbleName, giant, 0.06, 0.02, Nodes);
				}
			}

			if (BreastL03 && BreastR03) {
				GRumble::Once("BreastDot_L", giantref, 0.06, 0.025, "L Breast03");
				GRumble::Once("BreastDot_R", giantref, 0.06, 0.025, "R Breast03");
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastL03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastR03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				GRumble::Once("BreastDot_L", giantref, 0.06, 0.025, "NPC L Breast");
				GRumble::Once("BreastDot_R", giantref, 0.06, 0.025, "NPC R Breast");
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastL, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastR, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			}
			return false;
		});
	}

	void StopDamageOverTime(Actor* giant) {
		std::string name = std::format("BreastDOT_{}", giant->formID);
		TaskManager::Cancel(name);
	}

	void LayingStaminaDrain_Launch(Actor* giant) {
		std::string name = std::format("LayingDrain_{}", giant->formID);
		auto gianthandle = giant->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			float stamina = GetAV(giantref, ActorValue::kStamina);
			DamageAV(giantref, ActorValue::kStamina, 0.12 * GetButtCrushCost(giant));

			if (!IsButtCrushing(giantref)) {
				return false;
			}
			return true;
		});
	}

	void LayingStaminaDrain_Cancel(Actor* giant) {
		std::string name = std::format("LayingDrain_{}", giant->formID);
		TaskManager::Cancel(name);
	}

	void InflictBodyDamage(Actor* giant) {
		float damage = GetBoobCrushDamage(giant);
		float perk = GetPerkBonus_Basics(giant);
		for (auto Nodes: BODY_NODES) {
			auto Node = find_node(giant, Nodes);
			if (Node) {
				std::string rumbleName = std::format("Node: {}", Nodes);
				DoDamageAtPoint(giant, Radius_BreastCrush_BodyImpact, Damage_BreastCrush_Body * damage, Node, 400, 0.10, 0.85, DamageSource::BodyCrush);
				DoLaunch(giant, 1.20 * perk, 4.20, Node);
				GRumble::Once(rumbleName, giant, 1.00 * damage, 0.02, Nodes);
			}
		}
	}

	void InflictBreastDamage(Actor* giant) {
		float damage = GetBoobCrushDamage(giant);

		float perk = GetPerkBonus_Basics(giant);
		float dust = 1.0;

		InflictBodyDamage(giant);

		if (HasSMT(giant)) {
			dust = 1.25;
		}

		auto BreastL = find_node(giant, "NPC L Breast");
		auto BreastR = find_node(giant, "NPC R Breast");
		auto BreastL03 = find_node(giant, "L Breast03");
		auto BreastR03 = find_node(giant, "R Breast03");

		if (BreastL03 && BreastR03) {
			DoDamageAtPoint(giant, Radius_BreastCrush_BreastImpact, Damage_BreastCrush_Impact * damage, BreastL03, 4, 0.70, 0.85, DamageSource::BreastImpact);
			DoDamageAtPoint(giant, Radius_BreastCrush_BreastImpact, Damage_BreastCrush_Impact * damage, BreastR03, 4, 0.70, 0.85, DamageSource::Breast);
			DoDustExplosion(giant, 1.25 * dust + damage/10, FootEvent::Right, "L Breast03");
			DoDustExplosion(giant, 1.25 * dust + damage/10, FootEvent::Left, "R Breast03");
			DoFootstepSound(giant, 1.25, FootEvent::Right, "R Breast03");
			DoFootstepSound(giant, 1.25, FootEvent::Left, "L Breast03");
			DoLaunch(giant, 1.20 * perk, 4.20, FootEvent::Breasts);
			GRumble::Once("Breast_L", giant, 1.20 * damage, 0.02, "L Breast03");
			GRumble::Once("Breast_R", giant, 1.20 * damage, 0.02, "R Breast03");
			ModGrowthCount(giant, 0, true); // Reset limit
			return;
		} else if (BreastL && BreastR) {
			DoDamageAtPoint(giant, Radius_BreastCrush_BreastImpact, Damage_BreastCrush_Impact * damage, BreastL, 4, 0.70, 0.85, DamageSource::BreastImpact);
			DoDamageAtPoint(giant, Radius_BreastCrush_BreastImpact, Damage_BreastCrush_Impact * damage, BreastR, 4, 0.70, 0.85, DamageSource::BreastImpact);
			DoDustExplosion(giant, 1.25 * dust + damage/10, FootEvent::Right, "NPC L Breast");
			DoDustExplosion(giant, 1.25 * dust + damage/10, FootEvent::Left, "NPC R Breast");
			DoFootstepSound(giant, 1.25, FootEvent::Right, "NPC R Breast");
			DoFootstepSound(giant, 1.25, FootEvent::Right, "NPC L Breast");
			DoLaunch(giant, 1.20 * perk, 4.20, FootEvent::Breasts);
			GRumble::Once("Breast_L", giant, 1.20 * damage, 0.02, "NPC L Breast");
			GRumble::Once("Breast_R", giant, 1.20 * damage, 0.02, "NPC R Breast");
			ModGrowthCount(giant, 0, true); // Reset limit
			return;
		} else {
			if (!BreastR) {
				Notify("Error: Missing Breast Nodes"); // Will help people to troubleshoot it. Not everyone has 3BB/XPMS32 body.
				Notify("Error: effects not inflicted");
				Notify("Suggestion: install Female body replacer");
			} else if (!BreastR03) {
				Notify("Error: Missing 3BB Breast Nodes"); // Will help people to troubleshoot it. Not everyone has 3BB/XPMS32 body.
				Notify("Error: effects not inflicted");
				Notify("Suggestion: install 3BB/SMP Body");
			}
		}
	}

	void GTS_BoobCrush_Smile_On(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 1.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 1.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 2, 1.0, "expression");
		//AdjustFacialExpression(giant, 0, 0.75, "phenome");
	}

	void GTS_BoobCrush_Smile_Off(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 2, 0.0, "expression");
		//AdjustFacialExpression(giant, 0, 0.0, "phenome");
	}

	void GTS_BoobCrush_DOT_Start_Loop(AnimationEventData& data) {
		StartDamageOverTime(&data.giant);
	}

	void GTS_BoobCrush_TrackBody(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::Breasts_02);
	}
	void GTS_BoobCrush_UnTrackBody(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::Breasts_02);
	}
	void GTS_BoobCrush_BreastImpact(AnimationEventData& data) {
		InflictBreastDamage(&data.giant);
	}
	void GTS_BoobCrush_DOT_Start(AnimationEventData& data) {
		LayingStaminaDrain_Launch(&data.giant);
	}
	void GTS_BoobCrush_DOT_End(AnimationEventData& data) {
		auto giant = &data.giant;
		StopDamageOverTime(giant);
		ModGrowthCount(giant, 0, true);
		LayingStaminaDrain_Cancel(giant);
	}
	void GTS_BoobCrush_Grow_Start(AnimationEventData& data) {
		auto giant = &data.giant;
		float bonus = 0.24 * (GetGrowthCount(giant) + 1.0);

		PlayMoanSound(giant, 1.0);
		ModGrowthCount(giant, 1.0, false);
		SetBonusSize(giant, bonus, false);
		SpringGrow_Free(giant, bonus, 0.3 / GetAnimationSlowdown(giant), "BreastCrushGrowth");

		float WasteStamina = 60.0 * GetButtCrushCost(giant);
		DamageAV(giant, ActorValue::kStamina, WasteStamina);

		//CameraFOVTask(giant, 1.0, 0.003);
		
		Runtime::PlaySoundAtNode("growthSound", giant, 1.0, 1.0, "NPC Pelvis [Pelv]");
		

		StartRumble("CleavageRumble", data.giant, 0.06, 0.60);
	}
	void GTS_BoobCrush_Grow_Stop(AnimationEventData& data) {
		StopRumble("CleavageRumble", data.giant);
	}

	void GTS_BoobCrush_LoseSize(AnimationEventData& data) {
		auto giant = &data.giant;
		SetBonusSize(giant, 0.0, true);
	}
}

namespace Gts
{
	void AnimationBoobCrush::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_BoobCrush_DOT_Start_Loop", "BoobCrush", GTS_BoobCrush_DOT_Start_Loop);
		AnimationManager::RegisterEvent("GTS_BoobCrush_Smile_On", "BoobCrush", GTS_BoobCrush_Smile_On);
		AnimationManager::RegisterEvent("GTS_BoobCrush_Smile_Off", "BoobCrush", GTS_BoobCrush_Smile_Off);
		AnimationManager::RegisterEvent("GTS_BoobCrush_TrackBody", "BoobCrush", GTS_BoobCrush_TrackBody);
		AnimationManager::RegisterEvent("GTS_BoobCrush_UnTrackBody", "BoobCrush", GTS_BoobCrush_UnTrackBody);
		AnimationManager::RegisterEvent("GTS_BoobCrush_BreastImpact", "BoobCrush", GTS_BoobCrush_BreastImpact);
		AnimationManager::RegisterEvent("GTS_BoobCrush_DOT_Start", "BoobCrush", GTS_BoobCrush_DOT_Start);
		AnimationManager::RegisterEvent("GTS_BoobCrush_DOT_End", "BoobCrush", GTS_BoobCrush_DOT_End);
		AnimationManager::RegisterEvent("GTS_BoobCrush_Grow_Start", "BoobCrush", GTS_BoobCrush_Grow_Start);
		AnimationManager::RegisterEvent("GTS_BoobCrush_Grow_Stop", "BoobCrush", GTS_BoobCrush_Grow_Stop);
		AnimationManager::RegisterEvent("GTS_BoobCrush_LoseSize", "BoobCrush", GTS_BoobCrush_LoseSize);
	}
}
