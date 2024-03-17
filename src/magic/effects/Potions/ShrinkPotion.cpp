#include "magic/effects/Potions/ShrinkPotion.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"


namespace Gts {
	std::string ShrinkPotion::GetName() {
		return "ShrinkPotion";
	}

	void ShrinkPotion::OnStart() {
		auto caster = GetCaster();
		auto player = PlayerCharacter::GetSingleton();
		if (!caster || !player) {
			return;
		}

		GRumble::Once("ShrinkPotion", caster, 2.0, 0.05);

		float Volume = clamp(0.15, 2.0, get_visual_scale(caster)/8);
		Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0, "NPC Pelvis [Pelv]");
		log::info("Growth Potion start actor: {}", caster->GetDisplayFullName());
	}

	void ShrinkPotion::OnUpdate() {
		const float BASE_POWER = 0.000480;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float AlchemyLevel = clamp(1.0, 2.0, caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlchemy)/100 + 1.0);
		GRumble::Once("ShrinkPotion", caster, 0.4, 0.05);

		float Power = BASE_POWER * get_visual_scale(caster) * AlchemyLevel;

		if (get_target_scale(caster) > 0.12) {
			ShrinkActor(caster, Power, 0.0);
		} else {
			set_target_scale(caster, 0.12);
		}
	}

	void ShrinkPotion::OnFinish() {
	}
}
