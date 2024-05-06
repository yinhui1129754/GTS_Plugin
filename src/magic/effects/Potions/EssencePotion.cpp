#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/Potions/EssencePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"

// A potion that increases max possible size

namespace {
    void shake_screen_do_moan(Actor* giant, float power) {
		if (power >= 0.07) {
			bool Blocked = IsActionOnCooldown(giant, CooldownSource::Emotion_Moan);
			if (!Blocked) {
				PlayMoanSound(giant, 1.0);
				ApplyActionCooldown(giant, CooldownSource::Emotion_Moan);
			}
		}

		if (giant->formID == 0x14) {
			float shake = 12.0 * power;
			shake_camera(giant, 0.50 * shake, 0.38 * shake);
		}
    }
}

namespace Gts {
	std::string EssencePotion::GetName() {
		return "EssencePotion";
	}

    EssencePotion::EssencePotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionWeak")) {
			this->power = 0.02;
		} else if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionNormal")) {
			this->power = 0.04;
		} else if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionStrong")) {
			this->power = 0.06;
		} else if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionExtreme")) {
			this->power = 0.08; 
		} 
	}

	void EssencePotion::OnStart() {
		auto caster = GetCaster();

		if (caster) { // player exclusive
			if (caster->formID == 0x14) {
				float scale = get_visual_scale(caster);

				TESGlobal* BonusSize = Runtime::GetGlobal("ExtraPotionSize"); 
				// Bonus size is added on top of all size calculations through this global
				// Applied inside GtsManager.cpp (script)
				if (BonusSize) {
					BonusSize->value += this->power/1.82; // convert to m
				}

				SpawnCustomParticle(caster, ParticleType::Red, NiPoint3(), "NPC COM [COM ]", scale * (this->power * 25)); // Just some nice visuals
				shake_screen_do_moan(caster, this->power);
			}
			Potion_Penalty(caster);
        }
	}
}