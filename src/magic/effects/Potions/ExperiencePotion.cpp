#include "magic/effects/Potions/ExperiencePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"

// A potion that gives size experience

namespace Gts {
	std::string ExperiencePotion::GetName() {
		return "ExperiencePotion";
	}

	void ExperiencePotion::OnStart() {
		auto caster = GetCaster();

		if (caster) {
            int RNG = rand() % 30;
            ModSizeExperience(caster, 1.0 * (1.0 + (RNG * 0.01)));
			Potion_Penalty(caster);

			if (caster->formID == 0x14) {
				shake_camera(caster, 0.50, 0.50);
			}

			float scale = get_visual_scale(caster);

			SpawnCustomParticle(caster, ParticleType::Red, NiPoint3(), "NPC COM [COM ]", scale * 2.0); // Just some nice visuals
        }
	}
}


