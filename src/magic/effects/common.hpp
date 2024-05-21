#pragma once
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/height.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"

#include "events.hpp"
#include "node.hpp"
// Module that handles various magic effects

namespace {
	const float MASTER_POWER = 2.0;
}

namespace Gts {
	inline float TimeScale() {
		const float BASE_FPS = 60.0; // Parameters were optimised on this fps
		return Time::WorldTimeDelta() * BASE_FPS;
	}

	inline bool CanBendLifeless(Actor* giant) {
		bool allow = Runtime::HasPerkTeam(giant, "BendLifeless");
		return allow;
	}

	inline void AdvanceSkill(Actor* giant, ActorValue Attribute, float points, float multiplier) {
		// DLL Equivalent of AdvanceSkill from Papyrus, does the same thing
		if (giant->formID == 0x14) {
			//log::info("Advancing skill, points: {}, Mult: {}, TimeScale: {}, Result: {}, * 60: {}", points, multiplier, TimeScale(), points * multiplier * TimeScale(), points * 60 * multiplier * TimeScale());
			//float Level = GetAV(giant, Attribute) + 1.0;
			//log::info("Level: {}", Level);
			giant->UseSkill(Attribute, points * 20 * multiplier * TimeScale(), nullptr);
		}
	}

	inline void Potion_Penalty(Actor* giant) { // Normal people just die if they drink them
		if (giant->formID != 0x14 && !IsTeammate(giant)) {
			float currentscale = get_visual_scale(giant);
			update_target_scale(giant, -currentscale * 0.5, SizeEffectType::kNeutral);
			giant->KillImmediate();
		}
	}

	inline void AdjustSizeReserve(Actor* giant, float value) {
		if (!Runtime::HasPerk(giant, "SizeReserve")) {
			return;
		}
		auto Cache = Persistent::GetSingleton().GetData(giant);
		if (Cache) {
			Cache->SizeReserve += value;
		}
	}

	inline float Shrink_GetPower(Actor* giant, Actor* tiny) { // for shrinking another
		float reduction = 1.0 / GetSizeFromBoundingBox(tiny);
		//log::info("Default Shrink power for {} is {}", tiny->GetDisplayFullName(), reduction);
		if (IsUndead(tiny, false) && !IsLiving(tiny)) {
			if (CanBendLifeless(giant)) {
				reduction *= 0.31;
			} else {
				reduction *= 0.22;
			}
		} else if (IsGiant(tiny)) {
			reduction *= 0.75;
		} else if (IsMechanical(tiny)) {
			if (CanBendLifeless(giant)) {
				reduction *= 0.12;
			} else {
				reduction *= 0.0;
			}
		}
		//log::info("Total Shrink power for {} is {}", tiny->GetDisplayFullName(), reduction);
		return reduction;
	}

	inline float SizeSteal_GetPower(Actor* giant, Actor* tiny) { // for gaining size
		float increase = GetSizeFromBoundingBox(tiny);
		if (IsUndead(tiny, false) && !IsLiving(tiny)) {
			if (CanBendLifeless(giant)) {
				increase *= 0.31;
			} else {
				increase *= 0.22;
			}
		} else if (IsMechanical(tiny)) {
			increase *= 0.0;
		}
		return increase;
	}

	inline void ModSizeExperience(Actor* Caster, float value) { // Adjust Matter Of Size skill
		if (value > 0) {
			bool Teammate = IsTeammate(Caster);
			if (Caster->formID == 0x14 || Teammate) {
				if (Teammate) {
					value *= 0.2;
				}
				auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
				auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
				auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");

				int random = (100 + (rand()% 25 + 1)) / 100;

				if (GtsSkillLevel->value >= 100.0) {
					GtsSkillLevel->value = 100.0;
					GtsSkillRatio->value = 0.0;
					return;
				}

				float skill_level = GtsSkillLevel->value;

				float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.10, 1.0);

				float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
				float Total = (value * random) * ValueEffectiveness;
				GtsSkillRatio->value += Total * GetXpBonus();

				if (GtsSkillRatio->value >= 1.0) {
					float transfer = std::clamp(Total - oldvaluecalc, 0.0f, 1.0f);
					GtsSkillRatio->value = transfer;
					GtsSkillLevel->value = skill_level + 1.0;
					GtsSkillProgress->value = GtsSkillLevel->value;
					AddPerkPoints(GtsSkillLevel->value);
				}
			}
		}
	}

	inline void ModSizeExperience_Crush(Actor* giant, Actor* tiny, bool check) {
		float size = get_visual_scale(tiny);
		float xp = 0.20 + (size * 0.02);
		if (tiny->IsDead() && check) {
			//Cprint("Crush Tiny is ded");
			xp *= 0.20;
		}
		ModSizeExperience(giant, xp); // Adjust Size Matter skill
	}

	inline void AdjustSizeLimit(float value, Actor* caster) {  // A function that adjusts Size Limit (Globals)
		if (caster->formID != 0x14) {
			return;
		}
		float progressionMultiplier = Persistent::GetSingleton().progression_multiplier;

		auto globalMaxSizeCalc = Runtime::GetFloat("GlobalMaxSizeCalc");
		if (globalMaxSizeCalc < 10.0) {
			Runtime::SetFloat("GlobalMaxSizeCalc", globalMaxSizeCalc + (value * 1.45 * 50 * progressionMultiplier * TimeScale())); // Always apply it
		}
	}

	inline void AdjustMassLimit(float value, Actor* caster) { // Adjust Size Limit for Mass Based Size Mode
		if (caster->formID != 0x14) {
			return;
		}
		auto selectedFormula = Runtime::GetInt("SelectedSizeFormula");
		float progressionMultiplier = Persistent::GetSingleton().progression_multiplier;
		if (selectedFormula) {
			if (selectedFormula >= 1.0) {
				SoftPotential mod {
					.k = 0.070,
					.n = 3,
					.s = 0.54,
				};
				auto globalMassSize = Runtime::GetFloat("GtsMassBasedSize");
				float modifier = soft_core(globalMassSize, mod);
				if (modifier <= 0.10) {
					modifier = 0.10;
				}
				value *= 10.0 * modifier;
				//log::info("Modifier: {}", modifier);
				auto sizeLimit = Runtime::GetFloat("sizeLimit");
				if (Runtime::HasPerk(caster, "ColossalGrowth")) {
					sizeLimit = 999999.0;
				}
				if (globalMassSize + 1.0 < sizeLimit) {
					Runtime::SetFloat("GtsMassBasedSize", globalMassSize + value * progressionMultiplier * TimeScale());
				}
			}
		}
	}

	inline float CalcEffeciency(Actor* caster, Actor* target) {
		float level_caster = caster->GetLevel();
		float level_target = target->GetLevel();
		float casterlevel = std::clamp(level_caster, 1.0f, 500.0f);
		float targetlevel = std::clamp(level_target, 1.0f, 500.0f);

		float SizeHunger = 1.0 + Ench_Hunger_GetPower(caster);

		float Gigantism_Caster = 1.0 + (Ench_Aspect_GetPower(caster) * 0.25); // get GTS Aspect Of Giantess
		float Gigantism_Target = 1.0 + Ench_Aspect_GetPower(target);  // get Tiny Aspect Of Giantess
		float efficiency = std::clamp(casterlevel/targetlevel, 0.50f, 1.0f);

		float Scale_Resistance = std::clamp(get_visual_scale(target), 1.0f, 9999.0f); // Calf_power makes shrink effects stronger based on scale, this fixes that.

		efficiency *= Potion_GetShrinkResistance(target);

		efficiency *= Gigantism_Caster * SizeHunger; // amplity it by Aspect Of Giantess (on gts) and size hunger potion bonus
		efficiency /= Gigantism_Target; // resistance from Aspect Of Giantess (on Tiny)
		efficiency /= Scale_Resistance;

		efficiency *= Shrink_GetPower(caster, target);// take bounding box of actor into account

		//log::info("efficiency between {} and {} is {}", caster->GetDisplayFullName(), target->GetDisplayFullName(), efficiency);

		return efficiency;
	}

	inline float CalcPower(Actor* actor, float scale_factor, float bonus, bool shrink) {
		float progress_mult = Persistent::GetSingleton().progression_multiplier;
		float size_cap = 0.5;
		// y = mx +c
		// power = scale_factor * scale + bonus
		if (shrink) { // allow for more size weakness when we need it
			size_cap = 0.02; // up to 98% shrink weakness
		}
		float scale = std::clamp(get_visual_scale(actor), size_cap, 999999.0f);
		return (scale * scale_factor + bonus) * progress_mult * MASTER_POWER * TimeScale();
	}

	inline void Grow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		update_target_scale(actor, CalcPower(actor, scale_factor, bonus, false), SizeEffectType::kGrow);
	}

	inline void ShrinkActor(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		update_target_scale(actor, -CalcPower(actor, scale_factor, bonus, true), SizeEffectType::kShrink);
	}

	inline bool Revert(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float amount = CalcPower(actor, scale_factor, bonus, false);
		float target_scale = get_target_scale(actor);
		float natural_scale = get_natural_scale(actor, true); // get_neutral_scale(actor) 

		if (target_scale < natural_scale) { // NOLINT
			set_target_scale(actor, natural_scale); // Without GetScale multiplier
			return false;
		} else {
			update_target_scale(actor, -amount, SizeEffectType::kNeutral);
		}
		return true;
	}

	inline void Grow_Ally(Actor* from, Actor* to, float receiver, float caster) {
		float receive = CalcPower(from, receiver, 0, false);
		float lose = CalcPower(from, receiver, 0, false);
		float CasterScale = get_target_scale(from);
		if (CasterScale > 1.0) { // We don't want to scale the caster below this limit!
			update_target_scale(from, -lose, SizeEffectType::kShrink);
		}
		update_target_scale(to, receive, SizeEffectType::kGrow);
	}

	inline void Steal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency, ShrinkSource source) {
		effeciency = std::clamp(effeciency, 0.0f, 1.0f);
		float visual_scale = get_visual_scale(from);

		float amount = CalcPower(from, scale_factor, bonus, false);
		float amount_shrink = CalcPower(from, scale_factor, bonus, false);

		float shrink_amount = (amount*0.22);
		float growth_amount = (amount_shrink*0.33*effeciency) * SizeSteal_GetPower(to, from);

		ModSizeExperience(to, 0.14 * scale_factor * visual_scale * SizeSteal_GetPower(to, from));

		update_target_scale(from, -shrink_amount, SizeEffectType::kShrink);
		update_target_scale(to, growth_amount, SizeEffectType::kGrow);

		float XpMult = 1.0;

		if (from->IsDead()) {
			XpMult = 0.25;
		}

		if (source == ShrinkSource::Hugs) { // For hugs: quest: shrink by 2 and 5 meters worth of size in total (stage 1 / 2) 
			AdvanceQuestProgression(to, nullptr, QuestStage::HugSteal, shrink_amount, false); // Stage 1: steal 2 meters worth of size (hugs)
			AdvanceQuestProgression(to, nullptr, QuestStage::HugSpellSteal, shrink_amount, false); // Stage 2: steal 5 meters worth of size (spells/hugs)
		} else { // For spell shrink part of the quest
			AdvanceSkill(to, ActorValue::kAlteration, shrink_amount, XpMult); // Gain vanilla Alteration xp
			AdvanceQuestProgression(to, nullptr, QuestStage::HugSpellSteal, shrink_amount, false);
		}

		AddStolenAttributes(to, amount*effeciency);
	}

	inline void TransferSize(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smt, ShrinkSource source) {
		const float BASE_POWER = 0.0005;
		const float DUAL_CAST_BONUS = 2.0;
		const float SMT_BONUS = 1.25;
		float PERK_BONUS = 1.0;

		if (IsEssential(caster, target)) {
			return;
		}

		float target_scale = get_visual_scale(target); // used for xp only
		float caster_scale = get_visual_scale(caster); // used for xp only

		transfer_effeciency = std::clamp(transfer_effeciency, 0.0f, 1.0f); // Ensure we cannot grow more than they shrink

		power *= BASE_POWER * CalcEffeciency(caster, target);

		if (dual_casting) {
			power *= DUAL_CAST_BONUS;
		}

		if (smt) {
			power *= SMT_BONUS;
		}

		if (Runtime::HasPerkTeam(caster, "FastShrink")) {
			PERK_BONUS += 0.15;
		}
		if (Runtime::HasPerkTeam(caster, "LethalShrink")) {
			PERK_BONUS += 0.35;
		}

		power *= PERK_BONUS; // multiply power by perk bonuses

		AdjustSizeLimit(0.0300 * target_scale * power, caster);
		AdjustMassLimit(0.0040 * target_scale * power, caster);

		auto GtsSkillLevel = GetGtsSkillLevel(caster);

		float alteration_level_bonus = 0.0380 + (GtsSkillLevel * 0.000360); // + 100% bonus at level 100
		Steal(target, caster, power, power * alteration_level_bonus, transfer_effeciency, source);
	}

	inline bool ShrinkToNothing(Actor* caster, Actor* target) {
		float bbscale = GetSizeFromBoundingBox(target);
		float target_scale = get_visual_scale(target);

		float SHRINK_TO_NOTHING_SCALE = 0.06 / bbscale;
		if (!caster) {
			return false;
		}
		if (!target) {
			return false;
		}

		if (target_scale <= SHRINK_TO_NOTHING_SCALE && !Runtime::HasMagicEffect(target,"ShrinkToNothing") && !IsTeammate(target)) {
			if (!ShrinkToNothingManager::CanShrink(caster, target)) {
				return false;
			}

			if (!target->IsDead()) {
				if (IsGiant(target)) {
					AdvanceQuestProgression(caster, target, QuestStage::Giant, 1, false);
				} else {
					AdvanceQuestProgression(caster, target, QuestStage::ShrinkToNothing, 1, false);
				}
			} else {
				AdvanceQuestProgression(caster, target, QuestStage::ShrinkToNothing, 0.25, false);
			}

			AdjustSizeLimit(0.0060, caster);
			AdjustMassLimit(0.0060, caster);

			AdjustSizeReserve(caster, target_scale * bbscale/25);
			PrintDeathSource(caster, target, DamageSource::Shrinked);
			ShrinkToNothingManager::Shrink(caster, target);
			return true;
		}
		return false;
	}

	inline void CrushBonuses(Actor* caster, Actor* target) {
		float target_scale = get_visual_scale(target) * GetSizeFromBoundingBox(target);

		int Random = rand() % 8;
		if (Random >= 8 && Runtime::HasPerk(caster, "GrowthDesirePerk")) {
			PlayMoanSound(caster, 1.0);
		}

		if (caster->formID == 0x14) {
			AdjustSizeReserve(caster, target_scale/25);
			AdjustSizeLimit(0.0066 * target_scale, caster);
			AdjustMassLimit(0.0066 * target_scale, caster);
		}
	}
}
