#include "magic/effects/growth.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string Growth::GetName() {
		return "Growth";
	}

	void Growth::OnUpdate() {
		const float BASE_POWER = 0.00150;
		const float DUAL_CAST_BONUS = 2.0;


		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float GtsSkillLevel = GetGtsSkillLevel();

		float SkillMult = 1.0 + (GtsSkillLevel * 0.01);
		float HpRegen = GetMaxAV(caster, ActorValue::kHealth) * 0.00005;

		float bonus = 1.0;

		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(),"EffectSizeAmplifyPotion")) {
			bonus = get_visual_scale(caster) * 0.25 + 0.75;
		}

		float power = BASE_POWER * SkillMult;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("GrowthSpellAdept")) {
			power *= 1.33;
		} else if (base_spell == Runtime::GetMagicEffect("GrowthSpellExpert")) {
			power *= 1.75;
			caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}


		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}

		Grow(caster, 0.0, power * bonus);
	}
}
