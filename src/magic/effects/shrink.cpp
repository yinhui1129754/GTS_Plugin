#include "magic/effects/shrink.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "ActionSettings.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string Shrink::GetName() {
		return "Shrink";
	}

	void Shrink::OnUpdate() {
		const float BASE_POWER = 0.00360;
		const float DUAL_CAST_BONUS = 2.0;
		auto base_spell = GetBaseEffect();
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto GtsSkillLevel = GetGtsSkillLevel();

		float SkillMult = 1.0 + (GtsSkillLevel * 0.01);

		float power = BASE_POWER * SkillMult;

		float bonus = 1.0;
		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
			bonus = get_visual_scale(caster) * 0.25 + 0.75;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		if (get_target_scale(caster) > Minimum_Actor_Scale) {
			ShrinkActor(caster, 0.0, power * bonus);
		} else {
			set_target_scale(caster, Minimum_Actor_Scale);
		}
	}
}
