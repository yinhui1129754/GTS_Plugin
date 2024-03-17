#include "managers/damage/tinyCalamity.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "timer.hpp"


using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

// TODO move away from polling

namespace {
	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = GameSettingCollection::GetSingleton();
		std::string s_name(name);
		Setting* setting = ini_conf->GetSetting(s_name.c_str());
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}


	void ManagePerkBonuses(Actor* actor) {
		auto& SizeManager = SizeManager::GetSingleton();
		float BalancedMode = SizeManager::GetSingleton().BalancedMode();
		float gigantism = 1.0 + (Ench_Aspect_GetPower(actor) * 0.30);

		float BaseGlobalDamage = SizeManager::GetSingleton().GetSizeAttribute(actor, 0);
		float BaseSprintDamage = SizeManager::GetSingleton().GetSizeAttribute(actor, 1);
		float BaseFallDamage = SizeManager::GetSingleton().GetSizeAttribute(actor, 2);

		float ExpectedGlobalDamage = 1.0;
		float ExpectedSprintDamage = 1.0;
		float ExpectedFallDamage = 1.0;

		///Normal Damage
		if (Runtime::HasPerkTeam(actor, "Cruelty")) {
			ExpectedGlobalDamage += 0.15/BalancedMode;
		}
		if (Runtime::HasPerkTeam(actor, "RealCruelty")) {
			ExpectedGlobalDamage += 0.35/BalancedMode;
		}
		if (IsGrowthSpurtActive(actor)) {
			ExpectedGlobalDamage *= (1.0 + (0.35/BalancedMode));
		}
		if (Runtime::HasPerkTeam(actor, "MightOfGiants")) {
			ExpectedGlobalDamage *= 1.15; // +15% damage
		}

		///Sprint Damage
		if (Runtime::HasPerkTeam(actor, "SprintDamageMult1")) {
			ExpectedSprintDamage += 0.25/BalancedMode;
		}
		///Fall Damage
		if (Runtime::HasPerkTeam(actor, "MightyLegs")) {
			ExpectedFallDamage += 0.3/BalancedMode;
		}
		///Buff by enchantment
		ExpectedGlobalDamage *= gigantism;
		ExpectedSprintDamage *= gigantism;
		ExpectedFallDamage *= gigantism;

		if (BaseGlobalDamage != ExpectedGlobalDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedGlobalDamage, 0);
		}
		if (BaseSprintDamage != ExpectedSprintDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedSprintDamage, 1);
		}
		if (BaseFallDamage != ExpectedFallDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedFallDamage, 2);
		}
	}

	float JumpHeight(Actor* actor) {
		float bonus = std::max(get_giantess_scale(actor), 1.0f);
		return 76.0 * bonus;
	}

	// Todo unify the functions
	void UpdateActors(Actor* actor) {
		if (!actor) {
			return;
		}
		static Timer timer = Timer(0.05);
		static Timer jumptimer = Timer(0.50);
		float size = get_giantess_scale(actor);

		if (timer.ShouldRunFrame()) { // Run once per 0.05 sec
			ManagePerkBonuses(actor);

			if (actor->formID == 0x14) {
				TinyCalamity_BonusSpeed(actor); // Manages SMT bonuses
			}
		}
	}
}


namespace Gts {
	AttributeManager& AttributeManager::GetSingleton() noexcept {
		static AttributeManager instance;
		return instance;
	}

	std::string AttributeManager::DebugName() {
		return "AttributeManager";
	}

	void AttributeManager::Update() {
		for (auto actor: find_actors()) {
			UpdateActors(actor);
		}
	}


	void AttributeManager::OverrideSMTBonus(float Value) {
		auto ActorAttributes = Persistent::GetSingleton().GetData(PlayerCharacter::GetSingleton());
		if (ActorAttributes) {
			ActorAttributes->smt_run_speed = Value;
		}
	}

	float AttributeManager::GetAttributeBonus(Actor* actor, ActorValue av) {
		auto profiler = Profilers::Profile("Attributes: GetAttributeBonus");
		if (!actor) {
			return 1.0;
		}

		float BalancedMode = SizeManager::GetSingleton().BalancedMode();
		float scale = get_giantess_scale(actor);
		if (scale <= 0) {
			scale = 1.0;
		}
		switch (av) {
			case ActorValue::kHealth: {
				if (actor->formID == 0x14 && HasSMT(actor)) {
					scale += 1.0;
				}

				float resistance = std::clamp(1.0f / scale, 0.001f, 3.0f); // 0.001% as max resistance, -300% is a max vulnerability.

				return resistance;

			}
			case ActorValue::kCarryWeight: {
				float bonusCarryWeightMultiplier = Runtime::GetFloatOr("bonusCarryWeightMultiplier", 1.0);
				float power = (bonusCarryWeightMultiplier/BalancedMode);
				if (actor->formID == 0x14 && HasSMT(actor)) {
					scale += 3.0;
				}
				if (scale > 1.0) {
					return power*scale + 1.0 - power;
				} else {
					return 1.0; // Don't reduce it if scale is < 1.0
				}
			}
			case ActorValue::kSpeedMult: {
				// TODO: Rework to something more succient that garuentees 1xspeed@1xscale
				SoftPotential& MS_adjustment = Persistent::GetSingleton().MS_adjustment;
				scale = get_visual_scale(actor); // take real scale into account for MS, makes sense after all. Smaller = slower.
				float MS_mult = soft_core(scale, MS_adjustment);
				float MS_mult_limit = clamp(0.750, 1.0, MS_mult);
				float Multy = clamp(0.70, 1.0, MS_mult);
				float speed_mult_walk = soft_core(scale, this->speed_adjustment_walk);
				float bonusspeed = clamp(0.90, 1.0, speed_mult_walk);
				float PerkSpeed = 1.0;
				auto actorData = Persistent::GetSingleton().GetData(actor);
				float Bonus = 1.0;
				if (actorData) {
					Bonus = actorData->smt_run_speed;
				}
				if (Runtime::HasPerk(actor, "BonusSpeedPerk")) {
					PerkSpeed = clamp(0.80, 1.0, speed_mult_walk);
				}

				float power = 1.0 * (Bonus/2.2 + 1.0)/MS_mult/MS_mult_limit/Multy/bonusspeed/PerkSpeed;
				if (scale > 1.0) {
					return power;
				} else {
					return scale * (Bonus/2.2 + 1.0);
				}
			}
			case ActorValue::kAttackDamageMult: {
				if (actor->formID == 0x14 && HasSMT(actor)) {
					scale += 1.0;
				}
				float bonusDamageMultiplier = Runtime::GetFloatOr("bonusDamageMultiplier", 1.0);
				float damage_storage = 1.0 + ((bonusDamageMultiplier) * scale - 1.0);
				if (scale > 1.0) {
					return damage_storage;
				} else {
					return scale;
				}
			}
			case ActorValue::kJumpingBonus: {
				float power = 1.0;
				float defaultjump = 1.0 + (1.0 * (scale - 1) * power);
				if (scale > 1.0) {
					return defaultjump;
				} else {
					return scale;
				}
			}
			default: {
				return 1.0;
			}
		}
	}

	float AttributeManager::AlterGetAv(Actor* actor, ActorValue av, float originalValue) {
		float bonus = 1.0;

		auto& attributes = AttributeManager::GetSingleton();
		switch (av) {
			case ActorValue::kCarryWeight: {
				bonus = attributes.GetAttributeBonus(actor, av);
				auto transient = Transient::GetSingleton().GetData(actor);
				if (transient != nullptr) {
					transient->carryweight_boost = (originalValue * bonus) - originalValue;
				}
				break;
			}
		}

		return originalValue * bonus;
	}

	float AttributeManager::AlterGetBaseAv(Actor* actor, ActorValue av, float originalValue) {
		float finalValue = originalValue;

		switch (av) {
			case ActorValue::kHealth: {
				float bonus = 1.0;
				auto& attributes = AttributeManager::GetSingleton();
				float scale = get_giantess_scale(actor);
				if (scale <= 0) {
					scale = 1.0;
				}

				if (scale > 1.0) {
					bonus = attributes.GetAttributeBonus(actor, av);
				} else {
					//Linearly decrease such that:
					//at zero scale health=0.0
					bonus = scale;
				}
				float perkbonus = GetStolenAttributes_Values(actor, ActorValue::kHealth); // calc health from the perk bonuses
				//float tempav = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av); // Do temp boosts here too
				//float permav = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);  //Do perm boosts here too
				finalValue = originalValue + perkbonus; // add flat health on top
				//finalValue += perkbonus; // add health boost from perks on top. It is limited to boost = 2 * playerlevel.

				//if (actor->formID == 0x14) {
				//log::info("Health originalValue: {}", originalValue);
				//log::info("Health tempav: {}", tempav);
				//log::info("Health permav: {}", permav);
				//log::info("Health bonus: {}", bonus);
				//log::info("Health finalValue: {}", finalValue);
				auto transient = Transient::GetSingleton().GetData(actor);
				if (transient) {
					transient->health_boost = finalValue - originalValue;
				}
				return finalValue;
			}
			case ActorValue::kMagicka: {
				float perkbonus = GetStolenAttributes_Values(actor, ActorValue::kMagicka);
				finalValue = originalValue + perkbonus;
				return finalValue;
			}
			case ActorValue::kStamina: {
				float perkbonus = GetStolenAttributes_Values(actor, ActorValue::kStamina);
				finalValue = originalValue + perkbonus;
				return finalValue;
			}
		}
		return finalValue;
	}

	float AttributeManager::AlterSetBaseAv(Actor* actor, ActorValue av, float originalValue) {
		float finalValue = originalValue;

		switch (av) {
			case ActorValue::kHealth: {
				auto transient = Transient::GetSingleton().GetData(actor);
				if (transient) {
					float lastEdit = transient->health_boost;
					if (finalValue - lastEdit > 0.0) {
						finalValue -= lastEdit;
					}
				}
			}
		}

		return finalValue;
	}

	float AttributeManager::AlterGetPermenantAv(Actor* actor, ActorValue av, float originalValue) {
		return originalValue;
	}

	float AttributeManager::AlterGetAvMod(float originalValue, Actor* actor, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		return originalValue;
	}

	float AttributeManager::GetJumpHeight(Actor* actor) {
		return JumpHeight(actor);
	}

	float AttributeManager::AlterMovementSpeed(Actor* actor, const NiPoint3& direction) {
		float bonus = 1.0;
		if (actor) {
			auto& attributes = AttributeManager::GetSingleton();
			bonus = attributes.GetAttributeBonus(actor, ActorValue::kSpeedMult);
			float volume = 0.0;
		}
		return bonus;
	}
}
