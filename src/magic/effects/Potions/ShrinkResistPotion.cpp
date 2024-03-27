#include "managers/GtsManager.hpp"
#include "magic/effects/Potions/ShrinkResistPotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace {
	void PlayGrowthAudio(Actor* giant, bool checkTimer) {
		if (checkTimer) {
			GRumble::Once("ShrinkResistPotion", giant, 2.0, 0.05);
			float Volume = clamp(0.20, 2.0, get_visual_scale(giant)/10);
			Runtime::PlaySoundAtNode("growthSound", giant, Volume, 1.0, "NPC Pelvis [Pelv]");
		}
	}
}

namespace Gts {
	std::string ShrinkResistPotion::GetName() {
		return "ShrinkResistPotion";
	}

	ResistancePotion::ResistancePotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("ResistShrinkPotionWeak")) {
			this->Resistance = 0.2;
		} else if (base_spell == Runtime::GetMagicEffect("ResistShrinkPotionNormal")) {
			this->Resistance = 0.4;
		} else if (base_spell == Runtime::GetMagicEffect("ResistShrinkPotionStrong")) {
			this->Resistance = 0.6;
		} else if (base_spell == Runtime::GetMagicEffect("ResistShrinkPotionExtreme")) {
			this->Resistance = 0.8;
		} 
	}

	void ShrinkResistPotion::OnStart() {
		auto caster = GetCaster();
		if (caster) {
			Potion_SetShrinkResistance(this->Resistance);
            log::info("Setting shrink resistance to {}", this->Resistance);
		}
	}

	void ShrinkResistPotion::OnFinish() {
		auto caster = GetCaster();
		if (caster) {
			Potion_SetShrinkResistance(0.0);
		}
	}
}