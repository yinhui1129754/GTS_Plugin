
#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/restore_size.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace {
	void Task_RestoreSizeTask(Actor* caster, bool dual_casted) {

		float Power = 0.00065;

		if (dual_casted) {
			Power *= 2.0;
		}

		std::string name = std::format("RevertSize_{}", caster->formID);
		ActorHandle casterhandle = caster->CreateRefHandle();

		TaskManager::RunFor(name, 180.0, [=](auto& progressData) {
			if (!casterhandle) {
				return false;
			}
			auto casterref = casterhandle.get().get();

			bool BlockSound = IsActionOnCooldown(casterref, CooldownSource::Misc_RevertSound);

			if (!BlockSound) {
				float Volume = std::clamp(get_visual_scale(casterref) * 0.1f, 0.15f, 1.0f);
				ApplyActionCooldown(casterref, CooldownSource::Misc_RevertSound);
				Runtime::PlaySound("shrinkSound", casterref, Volume, 1.0);
			}

			Rumbling::Once("RestoreSizeOther", casterref, 0.6, 0.05);

			if (!Revert(casterref, Power, Power/2.5)) { // Terminate the task once revert size is complete
				return false;
			}
			return true;
		});
	}
}

namespace Gts {
	std::string RestoreSize::GetName() {
		return "RestoreSize";
	}

	void RestoreSize::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		float Volume = std::clamp(get_visual_scale(caster) * 0.1f, 0.10f, 1.0f);
		Runtime::PlaySound("shrinkSound", caster, Volume, 1.0);

		Task_RestoreSizeTask(caster, DualCasted());
	}
}
