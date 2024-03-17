#include "utils/actorUtils.hpp"
#include "utils/av.hpp"
#include "profiler.hpp"

using namespace RE;
using namespace SKSE;

namespace Gts {

	float GetMaxAV(Actor* actor, ActorValue av) {
		auto baseValue = actor->AsActorValueOwner()->GetBaseActorValue(av);
		auto permMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kPermanent, av);
		auto tempMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kTemporary, av);
		return baseValue + permMod + tempMod;
	}
	float GetAV(Actor* actor, ActorValue av) {
		// actor->GetActorValue(av); returns a cached value so we calc directly from mods
		float max_av = GetMaxAV(actor, av);
		auto damageMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kDamage, av);
		return max_av + damageMod;
	}
	void ModAV(Actor* actor, ActorValue av, float amount) {
		actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kTemporary, av, amount);
	}
	void SetAV(Actor* actor, ActorValue av, float amount) {
		float currentValue = GetAV(actor, av);
		float delta = amount - currentValue;
		ModAV(actor, av, delta);
	}

	void DamageAV(Actor* actor, ActorValue av, float amount) {
		if (IsInGodMode(actor) && amount > 0) { // do nothing if TGM is on and value is > 0
			return;
		}
		actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, -amount);
	}

	float GetPercentageAV(Actor* actor, ActorValue av) {
		if (av == ActorValue::kAttackDamageMult && actor->formID == 0x14) {
			/*log::info("AttackDamageMult For: {}", actor->GetDisplayFullName());
			   log::info("  - GetActorValueModifier:");
			   log::info("    - Damage: {}", actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kDamage, av));
			   log::info("    - Temp: {}", actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kTemporary, av));
			   log::info("    - Perm: {}", actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kPermanent, av));
			   log::info("  - Modifers:");
			   log::info("    - Damage {}: ", actor->GetActorRuntimeData().healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kDamage]);
			   log::info("    - Temp: {}", actor->GetActorRuntimeData().healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary]);
			   log::info("    - Perm: {}", actor->GetActorRuntimeData().healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kPermanent]);
			   log::info("  - Cpp:");
			   log::info("    - Value: {}", actor->AsActorValueOwner()->GetActorValue(av));
			   log::info("    - Base: {}", actor->AsActorValueOwner()->GetBaseActorValue(av));
			   log::info("    - Perm: {}", actor->AsActorValueOwner()->GetPermanentActorValue(av));
			 */
			//log::info("  - Papyrus:");
			/*log::info("    - Value: ", CallFunctionOn(actor, "Actor", "GetActorValue", "health"));
			   log::info("    - Base: ", CallFunctionOn(actor, "Actor", "GetBaseActorValue", "health"));
			   log::info("    - Percentage: ", CallFunctionOn(actor, "Actor", "GetActorValuePercentage", "health"));*/

		}
		return GetAV(actor, av)/GetMaxAV(actor, av);
	}

	void SetPercentageAV(Actor* actor, ActorValue av, float target) {
		double currentValue = GetAV(actor, av);
		double maxValue = GetMaxAV(actor, av);
		double percentage = currentValue/maxValue;
		double targetValue = target * maxValue;
		double delta = targetValue - currentValue;
		actor->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, av, delta);
	}

	float GetStaminaPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kStamina);
	}

	void SetStaminaPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kStamina, target);
	}

	float GetHealthPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kHealth);
	}

	void SetHealthPercentage(Actor* actor, float target) {
		auto profiler = Profilers::Profile("Av: SetHp%");
		SetPercentageAV(actor, ActorValue::kHealth, target);
	}

	float GetMagikaPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kMagicka);
	}

	void SetMagickaPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kMagicka, target);
	}

}
