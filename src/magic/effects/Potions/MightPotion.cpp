#include "magic/effects/Potions/MightPotion.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"

namespace Gts {
	std::string MightPotion::GetName() {
		return "MightPotion";
	}


	MightPotion::MightPotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("EffectMightPotionExtreme")) {
			this->Power = 0.45;
		} else if (base_spell == Runtime::GetMagicEffect("EffectMightPotionStrong")) {
			this->Power = 0.30;
		} else if (base_spell == Runtime::GetMagicEffect("EffectMightPotionNormal")) {
			this->Power = 0.20;
		} else if (base_spell == Runtime::GetMagicEffect("EffectMightPotionWeak")) {
			this->Power = 0.10;
		}
	}


	void MightPotion::OnStart() {
		auto caster = GetCaster();
		if (caster) {
			Potion_SetMightBonus(caster, this->Power, true); // Disallow potions to stack
			Potion_Penalty(caster);
		}
	}

	void MightPotion::OnFinish() {
		auto caster = GetCaster();
		if (caster) {
			Potion_SetMightBonus(caster, -this->Power, true);
		}
	}
}