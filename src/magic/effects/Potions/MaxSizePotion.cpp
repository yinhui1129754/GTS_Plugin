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
	float BonusMaxSize_Get(Actor* giant) {
		auto saved_data = Persistent::GetSingleton().GetData(giant);
		if (saved_data) {
			return saved_data->bonus_max_size;
		}
		return 1.0;
	}
	void BonusMaxSize_Modify(Actor* giant, float value, bool add) {
		auto saved_data = Persistent::GetSingleton().GetData(giant);
		if (saved_data) {
			if (!add) {
				saved_data->bonus_max_size = value;
			} else {
				saved_data->bonus_max_size += value;
			}
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
			this->Strenght = 0.35;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionStrong")) {
			this->Strenght = 0.20;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionNormal")) {
			this->Strenght = 0.15;
		} else if (base_spell == Runtime::GetMagicEffect("EffectSizePotionWeak")) {
			this->Strenght = 0.10;
		}
		//log::info("Strenght is {}", this->Strenght);
	}


	void MaxSizePotion::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float Gigantism = 1.0 + Ench_Aspect_GetPower(caster);
		float PotionPower = this->Strenght;
		float BonusSize = Runtime::GetFloat("sizeLimit") * PotionPower * Gigantism; // add % of scale, that's why we read sizeLimit value
		BonusMaxSize_Modify(caster, BonusSize, false);
	}

	void MaxSizePotion::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float PotionPower = this->Strenght;
		float Gigantism = 1.0 + Ench_Aspect_GetPower(caster);
		float expected = Runtime::GetFloat("sizeLimit") * PotionPower * Gigantism;
		if (saved_data) {
			if (saved_data->bonus_max_size < expected) {
				saved_data->bonus_max_size += expected; // Just to be sure
				return;
			}
		}
	}

	void MaxSizePotion::OnFinish() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto saved_data = Persistent::GetSingleton().GetData(caster);
		if (saved_data) {
			saved_data->bonus_max_size = 0;
		}
	}
}
