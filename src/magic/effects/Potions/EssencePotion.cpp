#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/Potions/EssencePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"

// A potion that increases max possible size

namespace {
    void do_moan(Actor* giant) {
        bool Blocked = IsActionOnCooldown(giant, CooldownSource::Emotion_Moan);
        if (!Blocked) {
            PlayMoanSound(giant, 1.0);
            ApplyActionCooldown(giant, CooldownSource::Emotion_Moan);
        }
        if (giant->formID == 0x14) {
            shake_camera(giant, 0.50, 0.33);
        }
    }

    void spawn_particle(Actor* giant, float scale) {
        auto node = find_node(giant, "NPC Root [Root]");
		log::info("Spawning particle");
		if (node) {
			NiPoint3 pos = node->world.translate;
			SpawnParticle(giant, 4.60, "GTS/Magic/Life_Drain.nif", NiMatrix3(), pos, scale, 7, nullptr);
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
			this->power = 0.01;
		} else if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionNormal")) {
			this->power = 0.015;
		} else if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionStrong")) {
			this->power = 0.02;
		} else if (base_spell == Runtime::GetMagicEffect("EffectEssencePotionExtreme")) {
			this->power = 0.03;
		} 
	}

	void EssencePotion::OnStart() {
		auto caster = GetCaster();

		if (caster) { // player exclusive
			if (caster->formID == 0x14) {
				float scale = get_visual_scale(caster);

				float BonusSize = Runtime::GetGlobal("ExtraPotionSize")->value;
				BonusSize += this->power;

				spawn_particle(caster, scale * (this->power * 30));
				do_moan(caster);
			}
			Potion_Penalty(caster);
        }
	}
}