#include "managers/GtsManager.hpp"
#include "magic/effects/Potions/GrowthPotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace {
	void PlayGrowthAudio(Actor* giant, bool checkTimer) {
		if (checkTimer) {
			GRumble::Once("GrowthPotion", giant, 2.0, 0.05);
			float Volume = clamp(0.20, 2.0, get_visual_scale(giant)/10);
			Runtime::PlaySoundAtNode("growthSound", giant, Volume, 1.0, "NPC Pelvis [Pelv]");
		}
	}
}

namespace Gts {
	std::string GrowthPotion::GetName() {
		return "GrowthPotion";
	}

	GrowPotion::GrowPotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("EffectSizePotionWeak")) {
			this->power = 0.5;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionNormal")) {
			this->power = 1.0;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionStrong")) {
			this->power = 1.5;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionExtreme")) {
			this->power = 2.0;
		} 
	}

	void GrowthPotion::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		if (this->power >= 2.0 && this->MoanTimer.ShouldRun()) {
			PlayMoanSound(caster, 0.44);
			if (caster->formID == 0x14) {
				shake_camera(caster, 0.50, 0.33);
			}
		}

		Potion_SetUnderGrowth(caster, true);
		PlayGrowthAudio(caster, true);
	}

	void GrowthPotion::OnUpdate() {
		float BASE_POWER = 0.000128 * this->power;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float AlchemyLevel = clamp(1.0, 2.0, caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlchemy)/100 + 1.0);
		GRumble::Once("GrowthPotion", caster, 0.4, 0.05);
		
		PlayGrowthAudio(caster, this->timer.ShouldRun());

		float HP = GetMaxAV(caster, ActorValue::kHealth) * 0.00035;
		caster->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HP * TimeScale());

		float Power = BASE_POWER * AlchemyLevel;

		Grow(caster, Power, 0.0);
		GRumble::Once("GrowButton", caster, 0.6, 0.05);
	}

	void GrowthPotion::OnFinish() {
		auto caster = GetCaster();
		if (caster) {
			Potion_SetUnderGrowth(caster, false);
		}
	}
}
