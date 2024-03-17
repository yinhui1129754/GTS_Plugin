#include "magic/effects/Potions/SizeStealPotion.hpp"
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

	}

	void SizeHunger::OnUpdate() {
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
