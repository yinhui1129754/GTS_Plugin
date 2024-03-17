#include "magic/effects/EnchGigantism.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "managers/GtsSizeManager.hpp"

namespace Gts {
	std::string Gigantism::GetName() {
		return "Gigantism";
	}

	void Gigantism::OnStart() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		this->magnitude = GetActiveEffect()->magnitude;
		float GigantismPower = this->magnitude;
		SizeManager::GetSingleton().ModEnchantmentBonus(caster, GigantismPower);
	}

	void Gigantism::OnUpdate() {
	}


	void Gigantism::OnFinish() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float GigantismPower = this->magnitude;
		SizeManager::GetSingleton().ModEnchantmentBonus(caster, -GigantismPower);
	}
}
