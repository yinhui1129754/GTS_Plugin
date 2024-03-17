#include "magic/effects/grow_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string GrowOther::GetName() {
		return "GrowOther";
	}

	void GrowOther::OnUpdate() {
		const float BASE_POWER = 0.00045;
		const float BONUS = 0.00045;
		const float SMT_BONUS = 2.0;
		const float DUAL_CAST_BONUS = 2.0;
		const float CRUSH_GROWTH_CHECK = 1.4;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float power = BASE_POWER;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("GrowAllyAdept")) {
			power *= 1.32;
		} else if (base_spell == Runtime::GetMagicEffect("GrowAllyExpert")) {
			power *= 1.75;
		}

		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);

		if (Runtime::GetFloat("CrushGrowthRate") >= CRUSH_GROWTH_CHECK) {
			power += BONUS;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		if (HasSMT(caster)) {
			power *= SMT_BONUS;
		}
		float Gain = power * ((caster_scale * 0.50 + 0.50) * target_scale);
		float Lose = power * (target_scale * 0.25);

		Grow_Ally(caster, target, Gain, Lose);
	}
}
