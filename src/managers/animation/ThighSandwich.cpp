// Animation: Stomp
//  - Stages
/*
   GTSSandwich_EnterAnim           // Animation was just fired
   GTSSandwich_MoveBody_start      // Rumble for entire body
   GTSSandwich_EnableRune          // Trigger the rune
   GTSSandwich_SitStart            // When sit start happens
   GTSSandwich_MoveBody_end        // Stop them (When body pretty much enters 'idle' state)
   GTSSandwich_MoveLL_start        // Left leg starts to move in space (When starting to crush)
   GTSSandwich_ThighImpact         // When Both thighs are supposed to deal damage to an actor (When 'Sandwiching') happens
   GTSSandwich_MoveLL_end          // Left leg isn't moving anymore (These 2 should be repeated)
   GTSSandwich_ThighLoop_Enter     // Enter Thigh Idle Loop

   GTSSandwich_ThighAttack_start   // When we trigger Thigh Attack

   GTSSandwich_ThighLoop_Exit      // Exit thigh idle loop
   GTSSandwich_DisableRune         // Remove Rune
   GTSSandwich_DropDown            // When actor starts to 'jump off' from Rune
   GTSSandwich_FootImpact          // When both feet collide with the ground
   GTSSandwich_ExitAnim            // Animation is over

   GTSBEH_ThighSandwich_Start
   GTSBEH_ThighSandwich_Attack
   GTSBEH_ThighSandwich_ExitLoop

   //AnimObjectA = Tiny
   //AnimObjectB = rune
 */
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "events.hpp"
#include "spring.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	const std::vector<std::string_view> BODY_NODES = { // used for body rumble
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

	const std::vector<std::string_view> L_LEG_NODES = {
		"NPC L Foot [Lft ]",
		"NPC L Toe0 [LToe]",
		"NPC L Calf [LClf]",
		"NPC L PreRearCalf",
		"NPC L FrontThigh",
		"NPC L RearCalf [RrClf]",
	};



	void AllowToBeCrushed(Actor* actor, bool toggle) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->can_be_crushed = toggle;
		}
	}

	void DoThighDamage(Actor* giant, Actor* tiny, float animSpeed, float mult, float sizemult) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(giant);
		auto& sizemanager = SizeManager::GetSingleton();
		float sizedifference = get_visual_scale(giant)/ (get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny));
		float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0), 1.0f, 999.0f);
		float damage = Damage_ThighSandwich_Impact * sizedifference * animSpeed * mult * normaldamage * GetPerkBonus_Thighs(giant);
		if (HasSMT(giant)) {
			damage *= 1.5;
		}
		InflictSizeDamage(giant, tiny, damage);

		float experience = std::clamp(damage/200, 0.0f, 0.20f);
		ModSizeExperience(giant, experience);

		float hp = GetAV(tiny, ActorValue::kHealth);
		if (damage > hp || hp <= 0) {
			ModSizeExperience_Crush(giant, tiny, true);
			
			CrushManager::GetSingleton().Crush(giant, tiny);
			
			PrintDeathSource(giant, tiny, DamageSource::ThighSandwiched);
			AdvanceQuestProgression(giant, tiny, 5, 1.0, false);
			auto node = find_node(giant, "NPC R FrontThigh");
			if (node) {
				Runtime::PlaySoundAtNode("GtsCrushSound", giant, 1.0, 1.0, node);
			} else {
				Runtime::PlaySound("GtsCrushSound", giant, 1.0, 1.0);
			}
			sandwichdata.Remove(tiny);
		}
	}

	void StartBodyRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: L_LEG_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopBodyRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: L_LEG_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}

	void StartLeftLegRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: L_LEG_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopLeftLegRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: L_LEG_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}

	void GTSSandwich_EnterAnim(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.EnableSuffocate(false);
	}
	void GTSSandwich_MoveBody_start(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		for (auto tiny: sandwichdata.GetActors()) {
			AllowToBeCrushed(tiny, false);
			SetBeingHeld(tiny, true);
			sandwichdata.MoveActors(true);
			DisableCollisions(tiny, &data.giant);
		}
		StartBodyRumble("BodyRumble", data.giant, 0.5, 0.25);
	}
	void GTSSandwich_EnableRune(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		auto& sizemanager = SizeManager::GetSingleton();
		ManageCamera(&data.giant, true, CameraTracking::Thigh_Sandwich); // Focus camera on AnimObjectA
		sandwichdata.EnableRuneTask(&data.giant, false); // Start Growing the Rune
	}
	void GTSSandwich_SitStart(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_Sandwich_Idle", "KillerThighs", true, 0.25);
	}
	void GTSSandwich_MoveBody_end(AnimationEventData& data) {
		StopBodyRumble("BodyRumble", data.giant);
	}
	void GTSSandwich_MoveLL_start(AnimationEventData& data) {
		data.stage = 1.0;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.66;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.66 + GetRandomBoost();
		}
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.EnableSuffocate(false);
		StartLeftLegRumble("LLSandwich", data.giant, 0.10, 0.12);
		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", true, 1.0);
	}

	void GTSSandwich_MoveLL_start_H(AnimationEventData& data) {
		data.stage = 1.0;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.66;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.66 + GetRandomBoost();
		}
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.EnableSuffocate(false);
		StartLeftLegRumble("LLSandwichHeavy", data.giant, 0.15, 0.15);
		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", true, 2.5);
	}

	void GTSSandwich_ThighImpact(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		Runtime::PlaySoundAtNode("ThighSandwichImpact", &data.giant, 1.0, 1.0, "AnimObjectB");
		sandwichdata.EnableSuffocate(true);
		GRumble::Once("ThighImpact", &data.giant, 0.4, 0.15, "AnimObjectA");
		for (auto tiny: sandwichdata.GetActors()) {
			DoThighDamage(&data.giant, tiny, data.animSpeed, 1.0, 1.0);
			tiny->NotifyAnimationGraph("ragdoll");
			AllowToBeCrushed(tiny, true);
		}
		//std::string message = std::format("You're able to attack targets with her thighs during thigh sandwich. Press LMB for Normal attack, press both RMB + LMB for strong attack. Additionally, you can hold LMB or LMB + RMB to increase attack speed, or RMB to decrease it. Attack speed directly affects attack damage.");
		//TutorialMessage(message, "SandwichControls");
		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", false, 1.0);
	}

	void GTSSandwich_ThighImpact_H(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		Runtime::PlaySoundAtNode("ThighSandwichImpact", &data.giant, 1.2, 1.0, "AnimObjectA");
		sandwichdata.EnableSuffocate(true);
		GRumble::Once("ThighImpact", &data.giant, 0.75, 0.15, "AnimObjectA");
		for (auto tiny: sandwichdata.GetActors()) {
			DoThighDamage(&data.giant, tiny, data.animSpeed, 2.2, 0.75);
			Attacked(tiny, &data.giant);
			tiny->NotifyAnimationGraph("ragdoll");
			AllowToBeCrushed(tiny, true);
		}
		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", false, 2.5);
	}

	void GTSSandwich_MoveLL_end(AnimationEventData& data) {
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		StopLeftLegRumble("LLSandwich", data.giant);
	}

	void GTSSandwich_MoveLL_end_H(AnimationEventData& data) {
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		StopLeftLegRumble("LLSandwichHeavy", data.giant);
	}

	void GTSSandwich_ThighLoop_Enter(AnimationEventData& data) {
	}

	void GTSSandwich_ThighLoop_Exit(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		ManageCamera(&data.giant, false, CameraTracking::Thigh_Sandwich);
		sandwichdata.EnableSuffocate(false);
		//sandwichdata.DisableRuneTask(&data.giant, false); // Disable Rune Growing
		sandwichdata.EnableRuneTask(&data.giant, true); // Launch Rune Shrinking
		//sandwichdata.OverideShrinkRune(0.0);
		for (auto tiny: sandwichdata.GetActors()) {
			SetBeingHeld(tiny, false);
			PushActorAway(&data.giant, tiny, 1.0);
			EnableCollisions(tiny);
		}
		sandwichdata.MoveActors(false);

		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", false, 2.5);
	}

	void GTSSandwich_ThighAttack_start(AnimationEventData& data) {
	}

	void GTSSandwich_DisableRune(AnimationEventData& data) {

	}

	void GTSSandwich_DropDown(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		for (auto tiny: sandwichdata.GetActors()) {
			AllowToBeCrushed(tiny, true);
		}
		
		sandwichdata.ReleaseAll();

		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", false, 2.5);
		DrainStamina(&data.giant, "StaminaDrain_Sandwich_Idle", "KillerThighs", false, 0.25);
	}

	void GTSSandwich_ExitAnim(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		for (auto tiny: sandwichdata.GetActors()) {
			SetBeingHeld(tiny, false);
			EnableCollisions(tiny);
		}
		DrainStamina(&data.giant, "StaminaDrain_Sandwich", "KillerThighs", false, 2.5);
		//sandwichdata.DisableRuneTask(&data.giant, false); // Disable Rune Growth
		//sandwichdata.DisableRuneTask(&data.giant, true); // Disable Rune Shrink
		ManageCamera(&data.giant, false, CameraTracking::Thigh_Sandwich); // Allow sandwich repeat
	}

	void GTSSandwich_FootImpact(AnimationEventData& data) {
		float perk = GetPerkBonus_Thighs(&data.giant);
		DoFootstepSound(&data.giant, 1.05, FootEvent::Right, RNode);
		DoFootstepSound(&data.giant, 1.05, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 2.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 2.0, FootEvent::Left, LNode);
		DoDamageEffect(&data.giant, 6.0 * perk, 1.6, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoDamageEffect(&data.giant, 6.0 * perk, 1.6, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoLaunch(&data.giant, 0.85 * perk, 2.2, FootEvent::Right);
		DoLaunch(&data.giant, 0.85 * perk, 2.2, FootEvent::Left);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		auto giant = &data.giant;
		ManageCamera(giant, false, CameraTracking::Thigh_Sandwich); // Un-Focus camera on AnimObjectA. Just to be Sure.
	}



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////E V E N T S
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void ThighSandwichEnterEvent(const InputEventData& data) {
		auto& Sandwiching = ThighSandwichController::GetSingleton();
		auto pred = PlayerCharacter::GetSingleton();
		if (IsCrawling(pred)) {
			return;
		}
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerk(pred, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(pred)/3);
		}
		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, numberOfPrey);
		for (auto prey: preys) {
			Sandwiching.StartSandwiching(pred, prey);
			auto node = find_node(pred, "GiantessRune", false);
			if (node) {
				node->local.scale = 0.01;
				update_node(node);
			}
		}
	}

	void ThighSandwichEnterEvent_Follower(const InputEventData& data) {
		Actor* pred = PlayerCharacter::GetSingleton();
		ForceFollowerAnimation(pred, FollowerAnimType::ThighSandwich);
	}


	void ThighSandwichAttackEvent(const InputEventData& data) {
		Actor* player = GetPlayerOrControlled();
		
		if (IsGtsBusy(player)) {
			float WasteStamina = 20.0;
			if (Runtime::HasPerk(player, "KillerThighs")) {
				WasteStamina *= 0.65;
			}
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("ThighAttack", player);
			} else {
				if (IsThighSandwiching(player)) {
					TiredSound(player, "You're too tired to perform thigh sandwich");
				}
			}
		}
	}

	void ThighSandwichHeavyAttackEvent(const InputEventData& data) {
		auto player = GetPlayerOrControlled();
		if (IsGtsBusy(player)) {
			float WasteStamina = 35.0;
			if (Runtime::HasPerk(player, "KillerThighs")) {
				WasteStamina *= 0.65;
			}
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("ThighAttack_Heavy", player);
			} else {
				if (IsThighSandwiching(player)) {
					TiredSound(player, "You're too tired to perform strong thigh sandwich");
				}
			}
		}
	}

	void ThighSandwichExitEvent(const InputEventData& data) {
		if (!IsFreeCameraEnabled()) {
			auto player = GetPlayerOrControlled();
			if (IsGtsBusy(player)) {
				AnimationManager::StartAnim("ThighExit", player);
			}
		}
	}
}

namespace Gts
{
	void AnimationThighSandwich::RegisterEvents() {
		InputManager::RegisterInputEvent("ThighSandwichEnter", ThighSandwichEnterEvent);
		InputManager::RegisterInputEvent("PlayerThighSandwichEnter", ThighSandwichEnterEvent_Follower);
		InputManager::RegisterInputEvent("ThighSandwichAttack", ThighSandwichAttackEvent);
		InputManager::RegisterInputEvent("ThighSandwichAttackHeavy", ThighSandwichHeavyAttackEvent);
		InputManager::RegisterInputEvent("ThighSandwichExit", ThighSandwichExitEvent);
		AnimationManager::RegisterEvent("GTSSandwich_ThighImpact", "ThighSandwich", GTSSandwich_ThighImpact);
		AnimationManager::RegisterEvent("GTSSandwich_ThighImpact_H", "ThighSandwich", GTSSandwich_ThighImpact_H);
		AnimationManager::RegisterEvent("GTSSandwich_DropDown", "ThighSandwich", GTSSandwich_DropDown);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_start", "ThighSandwich", GTSSandwich_MoveLL_start);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_start_H", "ThighSandwich", GTSSandwich_MoveLL_start_H);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_end", "ThighSandwich", GTSSandwich_MoveLL_end);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_end_H", "ThighSandwich", GTSSandwich_MoveLL_end_H);
		AnimationManager::RegisterEvent("GTSSandwich_EnterAnim", "ThighSandwich", GTSSandwich_EnterAnim);
		AnimationManager::RegisterEvent("GTSSandwich_MoveBody_start", "ThighSandwich", GTSSandwich_MoveBody_start);
		AnimationManager::RegisterEvent("GTSSandwich_EnableRune", "ThighSandwich", GTSSandwich_EnableRune);
		AnimationManager::RegisterEvent("GTSSandwich_SitStart", "ThighSandwich", GTSSandwich_SitStart);
		AnimationManager::RegisterEvent("GTSSandwich_MoveBody_end", "ThighSandwich", GTSSandwich_MoveBody_end);
		AnimationManager::RegisterEvent("GTSSandwich_ThighLoop_Enter", "ThighSandwich", GTSSandwich_ThighLoop_Enter);
		AnimationManager::RegisterEvent("GTSSandwich_FootImpact", "ThighSandwich", GTSSandwich_FootImpact);
		AnimationManager::RegisterEvent("GTSSandwich_DisableRune", "ThighSandwich", GTSSandwich_DisableRune);
		AnimationManager::RegisterEvent("GTSSandwich_ThighLoop_Exit", "ThighSandwich", GTSSandwich_ThighLoop_Exit);
		AnimationManager::RegisterEvent("GTSSandwich_ExitAnim", "ThighSandwich", GTSSandwich_ExitAnim);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "ThighSandwich", GTSBEH_Exit);
	}

	void AnimationThighSandwich::RegisterTriggers() {
		AnimationManager::RegisterTrigger("ThighEnter", "ThighSandwich", "GTSBEH_ThighSandwich_Start");
		AnimationManager::RegisterTrigger("ThighAttack", "ThighSandwich", "GTSBEH_ThighSandwich_Attack");
		AnimationManager::RegisterTrigger("ThighAttack_Heavy", "ThighSandwich", "GTSBEH_ThighSandwich_Attack_H");
		AnimationManager::RegisterTrigger("ThighExit", "ThighSandwich", "GTSBEH_ThighSandwich_ExitLoop");
	}

	AnimationThighSandwich& AnimationThighSandwich::GetSingleton() noexcept {
		static AnimationThighSandwich instance;
		return instance;
	}

	std::string AnimationThighSandwich::DebugName() {
		return "AnimationThighSandwich";
	}

	void AnimationThighSandwich::Update() {

	}
}
