#include "magic/effects/Potions/MaxSizePotion.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"

namespace {
	void BonusMaxSize_Modify(Actor* giant, float value) {
		auto saved_data = Persistent::GetSingleton().GetData(giant);
		if (saved_data) {
			saved_data->bonus_max_size += value;
		}
	}
}


namespace Gts {
	std::string MaxSizePotion::GetName() {
		return "MaxSizePotion";
	}


	MaxSizePotion::MaxSizePotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("EffectSizePotionExtreme")) {
			this->Power = 0.35;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionStrong")) {
			this->Power = 0.20;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionNormal")) {
			this->Power = 0.15;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionWeak")) {
			this->Power = 0.10;
		}
	}


	void MaxSizePotion::OnStart() {
		auto caster = GetCaster();
		if (caster) {
			BonusMaxSize_Modify(caster, this->Power);
			Potion_Penalty(caster);
		}
	}

	void MaxSizePotion::OnFinish() {
		auto caster = GetCaster();
		if (caster) {
			BonusMaxSize_Modify(caster, -this->Power);
		}
	}
}
