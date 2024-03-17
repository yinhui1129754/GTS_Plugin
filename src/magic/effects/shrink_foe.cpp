#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/shrink_foe.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"

namespace {
	void ResetShrinkWeakness(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->ShrinkWeakness = 1.0;
		}
	}
	void AddShrinkWeakness(Actor* tiny, float value) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->ShrinkWeakness += value;
		}
	}
	float GetShrinkWeakness(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			return transient->ShrinkWeakness;
		}
		return 1.0;
	}

	void DecreaseWeaknessTask(Actor* tiny) {
		float Start = Time::WorldTimeElapsed();
		std::string name = std::format("ShrinkDebuff_{}", tiny->formID);
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!tinyhandle) {
				return false;
			}
			float Finish = Time::WorldTimeElapsed();
			auto tinyref = tinyhandle.get().get();
			float timepassed = Finish - Start;
			if (timepassed >= 2.00) {
				AddShrinkWeakness(tinyref, -0.018 * TimeScale());
				if (GetShrinkWeakness(tinyref) <= 1.0) {
					ResetShrinkWeakness(tinyref);
					return false; // Cancel task
				}
				return true; // end it
			}
			return true;
		});
	}

	void CancelWeaknessTask(Actor* tiny) {
		std::string name = std::format("ShrinkDebuff_{}", tiny->formID);
		TaskManager::Cancel(name);
	}
}


namespace Gts {
	std::string ShrinkFoe::GetName() {
		return "ShrinkFoe";
	}

	ShrinkFoe::ShrinkFoe(ActiveEffect* effect) : Magic(effect) {
		const float SHRINK_POWER = 2.15; // Power = Shrink Power
		const float SHRINK_EFFIC = 0.16; // Efficiency = size steal efficiency.
		const float SHRINK_AOE_POWER = 2.40;
		const float SHRINK_AOE_EFFIC = 0.18;
		const float SHRINK_AOE_MASTER_POWER = 2.70;
		const float SHRINK_AOE_MASTER_EFFIC = 0.20;
		const float SHRINK_BOLT_POWER = 6.0;
		const float SHRINK_BOLT_EFFIC = 0.06;
		const float SHRINK_STORM_POWER = 12.0;
		const float SHRINK_STORM_EFFIC = 0.03;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("ShrinkEnemy")) {
			this->power = SHRINK_POWER;
			this->efficiency = SHRINK_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkEnemyAOE")) {
			this->power = SHRINK_AOE_POWER;
			this->efficiency = SHRINK_AOE_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkEnemyAOEMast")) {
			// ShrinkEnemyAOEMast
			this->power = SHRINK_AOE_MASTER_POWER;
			this->efficiency = SHRINK_AOE_MASTER_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkBolt")) {
			// ShrinkBolt
			this->power = SHRINK_BOLT_POWER;
			this->efficiency = SHRINK_BOLT_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkStorm")) {
			// ShrinkBolt
			this->power = SHRINK_STORM_POWER;
			this->efficiency = SHRINK_STORM_EFFIC;
		}
	}

	void ShrinkFoe::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float sizediff = GetSizeDifference(caster, target, true);
		if (this->power >= 18.00 && sizediff > 4.0) {
			StaggerActor(caster, target, 100.0f);
		}
		//CancelWeaknessTask(target);
	}

	void ShrinkFoe::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		if (caster == target) {
			return;
		}

		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}

		auto& Persist = Persistent::GetSingleton();
		float SizeDifference = 1.0;
		float bonus = 1.0;
		float balancemodebonus = 1.0;
		float shrink = this->power * 3.2;
		float gainpower = this->efficiency;
		auto actor_data = Persist.GetData(target);
		
		if (this->power >= 18.00) {
			if (actor_data) {
				actor_data->half_life = 0.25; // Faster shrink, less smooth.
			}
			SizeDifference = 1.0 / std::clamp((get_visual_scale(target) * GetSizeFromBoundingBox(target)), 0.25f, 1.0f);
		} else if (this->power >= 10.0) {
			if (actor_data) {
				actor_data->half_life = 0.50; // Faster shrink, less smooth.
			}
			SizeDifference = 1.0 / std::clamp((get_visual_scale(target) * GetSizeFromBoundingBox(target)), 0.50f, 1.0f);
		} else {
			if (actor_data) {
				actor_data->half_life = 1.0;
			}
		}

		if (target->IsDead()) {
			bonus = 2.5;
			gainpower *= 0.20;
		}

		if (caster->formID == 0x14 && SizeManager::GetSingleton().BalancedMode() == 2.0) { // This is checked only if Balance Mode is enabled.
			balancemodebonus = 0.5;
		}

		
		TransferSize(caster, target, IsDualCasting(), shrink * SizeDifference * bonus, gainpower * balancemodebonus, false, ShrinkSource::magic);

		Attacked(target, caster); // make it work like a hostile spell

		if (ShrinkToNothing(caster, target)) {
		}
	}

	void ShrinkFoe::OnFinish() {
		auto Caster = GetCaster();
		auto Target = GetTarget();
		//DecreaseWeaknessTask(Target);
		Task_TrackSizeTask(Caster, Target, "Spell");
	}
}
