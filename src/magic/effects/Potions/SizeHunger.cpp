#include "magic/effects/Potions/SizeHunger.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "managers/GtsSizeManager.hpp"

// A potion that amplifies size gain

namespace Gts {
	std::string SizeHunger::GetName() {
		return "SizeHunger";
	}

	void SizeHunger::OnStart() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float Power = GetActiveEffect()->magnitude;
		SizeManager::GetSingleton().SetSizeHungerBonus(caster, Power);
		Potion_Penalty(caster);

	}

	void SizeHunger::OnFinish() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float Power = SizeManager::GetSingleton().GetSizeHungerBonus(caster);
		SizeManager::GetSingleton().ModSizeHungerBonus(caster, -Power);
	}
}
