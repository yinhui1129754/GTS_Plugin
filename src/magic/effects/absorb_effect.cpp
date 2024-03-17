#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/absorb_effect.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"


#include "timer.hpp"

namespace Gts {
	Absorb::Absorb(ActiveEffect* effect) : Magic(effect) {}

	std::string Absorb::GetName() {
		return "Absorb";
	}

	void Absorb::OnStart() {
		auto target = GetTarget();
		if (!target) {
			return;
		} 
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		
		StaggerActor(caster, target, 0.25f * GetSizeDifference(caster, target, true));
		Attacked(target, caster);
		
	}

	void Absorb::OnUpdate() {
		const float SMT_BONUS = 1.0;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		if (target == caster) {
			return;
		}
		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}

		float size_difference = GetSizeDifference(caster, target, true);

		if (HasSMT(caster)) {
			size_difference += SMT_BONUS;
		} // More shrink with SMT

		
		if (size_difference >= 3.0) {
			size_difference = 3.0;
		} // Cap Size Difference

		float shrink_power = 2.5 * size_difference;
		float gain_size = 0.0025;

		if (target->IsDead()) {
			shrink_power *= 3.0;
			gain_size *= 0.20;
		}

		TransferSize(caster, target, true, shrink_power, gain_size, false, ShrinkSource::magic);
		Attacked(target, caster);// make it seen as hostile action

		if (ShrinkToNothing(caster, target)) { // chance to receive more size xp and grow even bigger
			AbsorbShout_BuffCaster(caster, target);
		}
	}
	

	void Absorb::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();
		Task_TrackSizeTask(caster, target, "Absorb");
	}
}
