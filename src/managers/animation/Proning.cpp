#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/Proning.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

namespace {

    const std::vector<std::string_view> BODY_NODES = {
		"NPC R Thigh [RThg]",
		"NPC L Thigh [LThg]",
		"NPC R Butt",
		"NPC L Butt",
		"NPC Spine [Spn0]",
		"NPC Spine1 [Spn1]",
		"NPC Spine2 [Spn2]",
	};

    void StartBodyDamage_DOT(Actor* giant) {
		float damage = 2.0 * TimeScale();
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("BodyDOT_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			auto BreastL = find_node(giantref, "NPC L Breast");
			auto BreastR = find_node(giantref, "NPC R Breast");
			auto BreastL03 = find_node(giantref, "L Breast03");
			auto BreastR03 = find_node(giantref, "R Breast03");

			if (!IsProning(giantref)) {
				return false;
			}

			for (auto Nodes: BODY_NODES) {
				auto Node = find_node(giantref, Nodes);
				if (Node) {
					DoDamageAtPoint(giant, Radius_Proning_BodyDOT, Damage_BreastCrush_BodyDOT * damage, Node, 400, 0.10, 1.33, DamageSource::BodyCrush);
				}
			}

			ApplyThighDamage(giant, true, false, Radius_ThighCrush_Idle, Damage_BreastCrush_BodyDOT * damage * 0.6, 0.10, 1.6, 200, DamageSource::ThighCrushed);
			ApplyThighDamage(giant, false, false, Radius_ThighCrush_Idle, Damage_BreastCrush_BodyDOT * damage * 0.6, 0.10, 1.6, 200, DamageSource::ThighCrushed);

			if (BreastL03 && BreastR03) {
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastL03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastR03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastL, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastR, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			}
			return false;
		});
	}

	void StartBodyDamage_Slide(Actor* giant) {
		float damage = 18.0 * TimeScale();
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("BodyDOT_Slide_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			auto BreastL = find_node(giantref, "NPC L Breast");
			auto BreastR = find_node(giantref, "NPC R Breast");
			auto BreastL03 = find_node(giantref, "L Breast03");
			auto BreastR03 = find_node(giantref, "R Breast03");

			if (!IsProning(giantref)) {
				return false;
			}

			for (auto Nodes: BODY_NODES) {
				auto Node = find_node(giantref, Nodes);
				if (Node) {
					std::string rumbleName = std::format("Node: {}", Nodes);
					DoDamageAtPoint(giant, Radius_BreastCrush_BodyDOT, Damage_BreastCrush_BodyDOT * damage, Node, 200, 0.10, 1.0, DamageSource::BodyCrush);
					GRumble::Once(rumbleName, giant, 0.25, 0.02, Nodes);
				}
			}

			ApplyThighDamage(giant, true, false, Radius_ThighCrush_Idle, Damage_BreastCrush_BodyDOT * damage * 0.6, 0.10, 1.0, 200, DamageSource::ThighCrushed);
			ApplyThighDamage(giant, false, false, Radius_ThighCrush_Idle, Damage_BreastCrush_BodyDOT * damage * 0.6, 0.10, 1.0, 200, DamageSource::ThighCrushed);

			if (BreastL03 && BreastR03) {
				GRumble::Once("BreastDot_L", giantref, 0.25, 0.025, "L Breast03");
				GRumble::Once("BreastDot_R", giantref, 0.25, 0.025, "R Breast03");
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastL03, 200, 0.10, 1.0, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastR03, 200, 0.10, 1.0, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				GRumble::Once("BreastDot_L", giantref, 0.25, 0.025, "NPC L Breast");
				GRumble::Once("BreastDot_R", giantref, 0.25, 0.025, "NPC R Breast");
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastL, 200, 0.10, 1.0, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, Radius_BreastCrush_BreastDOT, Damage_BreastCrush_BreastDOT * damage, BreastR, 200, 0.10, 1.0, DamageSource::BreastImpact);
				return true;
			}
			return false;
		});
	}

    void StopBodyDamage_Slide(Actor* giant) {
		std::string name = std::format("BodyDOT_Slide_{}", giant->formID);
		TaskManager::Cancel(name);
	}

    //////////////////////////////////////////////////////
    ///// E V E N T S
    //////////////////////////////////////////////////////

    void GTS_DiveSlide_ON(AnimationEventData& data) {
		auto giant = &data.giant;
		StartBodyDamage_Slide(giant);
	}
	void GTS_DiveSlide_OFF(AnimationEventData& data) {
		auto giant = &data.giant;
		StopBodyDamage_Slide(giant);
	}
	void GTS_BodyDamage_ON(AnimationEventData& data) {
		auto giant = &data.giant;
		SetProneState(giant, true);
		StartBodyDamage_DOT(giant);
	}
	void GTS_BodyDamage_Off(AnimationEventData& data) {
		auto giant = &data.giant;
		SetProneState(giant, false);
	}

	void SBOProneOnEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("SBO_ProneOn", player);
	}

	void SBOProneOffEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (player->IsSneaking()) {
			AnimationManager::StartAnim("SBO_ProneOff", player);
		}
	}

	void SBODiveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (player->IsSneaking()) {
			AnimationManager::StartAnim("SBO_Dive", player);
		}
	}
}

namespace Gts
{
	void AnimationProning::RegisterEvents() {
		InputManager::RegisterInputEvent("SBO_ToggleProne", SBOProneOnEvent);
		InputManager::RegisterInputEvent("SBO_DisableProne", SBOProneOffEvent);
		InputManager::RegisterInputEvent("SBO_ToggleDive", SBODiveEvent);

		AnimationManager::RegisterEvent("GTS_DiveSlide_ON", "Proning", GTS_DiveSlide_ON);
		AnimationManager::RegisterEvent("GTS_DiveSlide_OFF", "Proning", GTS_DiveSlide_OFF);
		AnimationManager::RegisterEvent("GTS_BodyDamage_ON", "Proning", GTS_BodyDamage_ON);
		AnimationManager::RegisterEvent("GTS_BodyDamage_Off", "Proning", GTS_BodyDamage_Off);
	}

	void  AnimationProning::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SBO_ProneOn", "Proning", "GTSBeh_ProneStart");
		AnimationManager::RegisterTrigger("SBO_ProneOff", "Proning", "GTSBeh_ProneStop");
		AnimationManager::RegisterTrigger("SBO_Dive", "Proning", "GTSBeh_ProneStart_Dive");
	}
}