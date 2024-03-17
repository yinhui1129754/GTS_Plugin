#include "managers/animation/AnimationManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/hitmanager.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "events.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void StaggerImmunity(Actor* attacker, Actor* receiver) {
		float sizedifference = GetSizeDifference(receiver, attacker, true);
		auto charCont = receiver->GetCharController();
		if (charCont) {
			receiver->SetGraphVariableFloat("GiantessScale", sizedifference); // Manages Stagger Resistance inside Behaviors.
		}
	}

	

	void TinyAsShield(Actor* receiver, float a_damage) {

		auto grabbedActor = Grab::GetHeldActor(receiver);
		if (!grabbedActor) {
			return;
		}
		if (IsTeammate(grabbedActor)) {
			return; // Don't kill teammates
		}

		DamageAV(grabbedActor, ActorValue::kHealth, a_damage * 0.50);
		if (grabbedActor->IsDead() || GetAV(grabbedActor, ActorValue::kHealth) < a_damage * 0.50) {
			if (!IsBetweenBreasts(receiver)) {
				PrintDeathSource(receiver, grabbedActor, DamageSource::BlockDamage);
			} else {
				PrintDeathSource(receiver, grabbedActor, DamageSource::Breast);
			}

			Grab::DetachActorTask(receiver);
			ModSizeExperience_Crush(receiver, grabbedActor, false);

			auto hand = find_node(receiver, "NPC L Hand [LHnd]");
			if (hand) {
				if (IsLiving(grabbedActor)) {
					SpawnParticle(receiver, 25.0, "GTS/Damage/Explode.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 5, 4, hand);
					SpawnParticle(receiver, 25.0, "GTS/Damage/Crush.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 5, 4, hand);
				} else {
					SpawnDustParticle(receiver, grabbedActor, "NPC L Hand [LHnd]", 3.0);
				}
			}
			CrushManager::Crush(receiver, grabbedActor);
			if (!LessGore()) {
				Runtime::PlaySoundAtNode("CrunchImpactSound", receiver, 1.0, 1.0, "NPC L Hand [LHnd]");
				Runtime::PlaySoundAtNode("CrunchImpactSound", receiver, 1.0, 1.0, "NPC L Hand [LHnd]");
				Runtime::PlaySoundAtNode("CrunchImpactSound", receiver, 1.0, 1.0, "NPC L Hand [LHnd]");
			} else {
				Runtime::PlaySoundAtNode("SoftHandAttack", receiver, 1.0, 1.0, "NPC L Hand [LHnd]");
			}
			GRumble::Once("GrabAttackKill", receiver, 8.0, 0.15, "NPC L Hand [LHnd]");
			AnimationManager::StartAnim("GrabAbort", receiver); // Abort Grab animation
			Grab::Release(receiver);
		}
	}

	void DropTinyChance(Actor* receiver, float damage, float scale) {
		static Timer DropTimer = Timer(0.33); // Check once per .33 sec
		float bonus = 1.0;
		if (Runtime::HasPerkTeam(receiver, "HugCrush_HugsOfDeath")) {
			return; // Full immunity
		}
		if (Runtime::HasPerkTeam(receiver, "HugCrush_Greed")) {
			bonus = 4.0; // 4 times bigger damage threshold to cancel hugs
		}
		if (Runtime::HasPerkTeam(receiver, "HugCrush_ToughGrip")) {
			float GetHP = GetHealthPercentage(receiver);
			if (GetHP >= 0.85) {
				return; // Drop only if hp is < 85%
			}
		}
		if (damage < 6.0 * bonus * scale) {
			return;
		}
		HugShrink::CallRelease(receiver); // Else release
	}

	void DoHitShake(Actor* receiver, float value) {
		if (IsFirstPerson()) {
			value *= 0.05;
		}
		GRumble::Once("HitGrowth", receiver, value, 0.15);
	}

	void HitGrowth(Actor* receiver, Actor* attacker, float GrowthValue, float SizeDifference, float BalanceMode) {
		//log::info("Growth Value: {}", GrowthValue);

		int LaughChance = rand() % 12;
		int ShrinkChance = rand() % 5;

		static Timer soundtimer = Timer(1.5);
		static Timer laughtimer = Timer(4.0);

		float Adjustment = 1.0 * GetSizeFromBoundingBox(attacker);

		DoHitShake(receiver, GrowthValue * 10);
		update_target_scale(receiver, GrowthValue, SizeEffectType::kShrink);
		
		if (soundtimer.ShouldRunFrame()) {
			Runtime::PlaySoundAtNode("growthSound", receiver, GrowthValue * 2, 1.0, "NPC Pelvis [Pelv]");
		}
		if (ShrinkChance >= 2) {
			update_target_scale(attacker, -GrowthValue/(6.0 * Adjustment*BalanceMode), SizeEffectType::kShrink); // Shrink Attacker
			update_target_scale(receiver, GrowthValue/(2.0 * Adjustment*BalanceMode), SizeEffectType::kGrow); // Grow receiver
			if (get_target_scale(attacker) <= 0.12/Adjustment) {
				set_target_scale(attacker, 0.12/Adjustment);
			}
		}
		if (SizeDifference >= 4.0 && LaughChance >= 11 && laughtimer.ShouldRunFrame()) {
			PlayLaughSound(receiver, 1.0, 1);
		}
	}

	void HitShrink(Actor* receiver, float ShrinkValue) {
		float scale = get_target_scale(receiver);
		float naturalscale = get_natural_scale(receiver);

		float lossmod = Runtime::GetFloatOr("SizeLossModifier", 1.0);
		float modifier = std::clamp(lossmod, 0.10f, 25.0f); // * size loss value

		ShrinkValue *= modifier;

		log::info("Shrink Value: {}", -ShrinkValue);

		if (scale < naturalscale) {
			set_target_scale(receiver, naturalscale); // reset to normal scale
			return;
		}
		update_target_scale(receiver, -ShrinkValue, SizeEffectType::kShrink);
	}

	void ApplyHitGrowth(Actor* attacker, Actor* receiver, float damage) {
		auto grabbedActor = Grab::GetHeldActor(receiver);
		if (grabbedActor == attacker) {
			return;
		}
		if (attacker == receiver) {
			return;
		}
		
		float scale = get_visual_scale(receiver);
		float naturalscale = get_natural_scale(receiver);
		
		auto& sizemanager = SizeManager::GetSingleton();
		float BalanceMode = sizemanager.BalancedMode();
		float SizeHunger = 1.0 + Ench_Hunger_GetPower(receiver);
		float Gigantism = 1.0 + Ench_Aspect_GetPower(receiver);
		float SizeDifference = get_visual_scale(receiver)/get_visual_scale(attacker);
		float DamageReduction = std::clamp(GetDamageResistance(receiver), 0.25f, 1.0f); // disallow going > than 1

		float resistance = 1.0;
		
		if (Runtime::HasMagicEffect(receiver, "ResistShrinkPotion")) {
			resistance = 0.25;
		}

		damage *= DamageReduction;

		if (receiver->formID == 0x14 && Runtime::HasPerk(receiver, "GrowthOnHitPerk") && sizemanager.GetHitGrowth(receiver) >= 1.0) { // if has perk
			float GrowthValue = std::clamp((-damage/2000) * SizeHunger * Gigantism, 0.0f, 0.25f * Gigantism);
			HitGrowth(receiver, attacker, GrowthValue, SizeDifference, BalanceMode);
			return;
		} else if (BalanceMode >= 2.0 && receiver->formID == 0x14 && !Runtime::HasPerk(receiver, "GrowthOnHitPerk")) { // Shrink us
			if (scale > naturalscale) {
				float sizebonus = std::clamp(get_visual_scale(attacker), 0.10f, 1.0f);
				float ShrinkValue = std::clamp(((-damage/850)/SizeHunger/Gigantism * sizebonus) * resistance, 0.0f, 0.25f / Gigantism); // affect limit by decreasing it
				HitShrink(receiver, ShrinkValue);
			}
		}
	}

	void ApplyToTinies(Actor* attacker, Actor* receiver, float damage) {
		float sizedifference = GetSizeDifference(receiver, attacker, true);
		DropTinyChance(receiver, -damage, sizedifference);
		TinyAsShield(receiver, -damage);
	}
}


namespace Gts {

	SizeHitEffects& SizeHitEffects::GetSingleton() noexcept {
		static SizeHitEffects instance;
		return instance;
	}

	std::string SizeHitEffects::DebugName() {
		return "SizeHitEffects";
	}

	void SizeHitEffects::ApplyEverything(Actor* attacker, Actor* receiver, float damage) {
		ApplyHitGrowth(attacker, receiver, damage);
		ApplyToTinies(attacker, receiver, damage);
		StaggerImmunity(attacker, receiver);
	}

	void SizeHitEffects::BreakBones(Actor* giant, Actor* tiny, float damage, int random) { // Used as a debuff
		if (tiny->IsDead()) {
			return;
		}
		if (!Runtime::HasPerkTeam(giant, "BoneCrusher")) {
			return;
		}
		int rng = (rand()% random + 1);
		if (rng <= 2 && random > 0) {
			float gs = get_visual_scale(giant);
			float ts = get_visual_scale(tiny);
			if (HasSMT(giant)) {
				gs += 3.0; // Allow to break bones with SMT
			}
			float sizediff = gs/ts;
			if (sizediff < 3.0) {
				return;
			}

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(-0.2, 0.2);
			if (!IsLiving(tiny)) {
				SpawnDustParticle(giant, tiny, "NPC Root [Root]", 1.0);
			} else {
				auto root = find_node(tiny, "NPC Root [Root]");
				if (root) {
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, ts * 1, 7, root);
				}
			}
			SizeManager::GetSingleton().ModSizeVulnerability(tiny, 0.05);
			InflictSizeDamage(giant, tiny, damage);
		}
	}
}
