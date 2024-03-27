#include "magic/effects/Potions/ExperiencePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"

// A potion that amplifies size gain

namespace Gts {
	std::string ExperiencePotion::GetName() {
		return "ExperiencePotion";
	}

	void ExperiencePotion::OnStart() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
        int RNG = rand() % 30;
		ModSizeExperience(caster, 145 * (1.0 + (RNG * 0.01)));
	}
}


