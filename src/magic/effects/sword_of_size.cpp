#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/sword_of_size.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace {
	float GetShrinkModifier(float value) {
		// https://www.desmos.com/calculator/ygoxbe7hjg
		float k = 0.9;
		float a = 0.0;
		float n = 0.7;
		float s = 1.0;
		float result = k*pow(s*(value-a), n);
		log::info("Shrink Result: {}, Value {}", result, value);
		if (result < 1.0) {
			log::info("value is < 1");
			return 1.0;
		}
		return result;
	}

	float Ench_AbsorbSize_GetPower(float power) {
		float power_result = 1.0;
		float reduction = GetShrinkModifier(power);
		power_result /= reduction;
		return power_result;
	}
}

namespace Gts {
	std::string SwordOfSize::GetName() {
		return "SwordOfSize";
	}

	SwordOfSize::SwordOfSize(ActiveEffect* effect) : Magic(effect) {
	}

	void SwordOfSize::OnStart() {
		auto caster = GetCaster();
		if (caster) {
			this->power = GetActiveEffect()->magnitude * 0.10;
		}
	}

	void SwordOfSize::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float gain_value = 0.02;
		float ench_power = this->power;

		float calc_power = ench_power * Ench_AbsorbSize_GetPower(ench_power);
		float base_shrink = std::clamp(calc_power, 1.0f, 10.0f);
		float shrink_value = base_shrink * 3;

		// balanced around default value of 3.0 

		if (target->IsDead()) {
			shrink_value *= 3.0;
			gain_value *= 0.20;
		}

		log::info("Shrink Value: {}", shrink_value);

		TransferSize(caster, target, false, shrink_value, gain_value, false, ShrinkSource::magic);
	}

	void SwordOfSize::OnFinish() {
		auto Caster = GetCaster();
		auto Target = GetTarget();
		Task_TrackSizeTask(Caster, Target, "Sword");
	}
}

