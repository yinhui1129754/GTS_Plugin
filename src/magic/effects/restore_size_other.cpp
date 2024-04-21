
#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/restore_size_other.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"


namespace {
	void Task_RestoreSizeTask(Actor* caster, Actor* target, bool dual_casted) {

		float Power = 0.00065;

		if (dual_casted) {
			Power *= 2.0;
		}

		std::string name = std::format("RevertSize_{}_{}", caster->formID, target->formID);
		ActorHandle targethandle = target->CreateRefHandle();

		TaskManager::RunFor(name, 180.0, [=](auto& progressData) {
			if (!targethandle) {
				return false;
			}
			auto targetref = targethandle.get().get();

			bool BlockSound = IsActionOnCooldown(targetref, CooldownSource::Misc_RevertSound);

			if (!BlockSound) {
				float Volume = std::clamp(get_visual_scale(targetref) * 0.1f, 0.15f, 1.0f);
				ApplyActionCooldown(targetref, CooldownSource::Misc_RevertSound);
				Runtime::PlaySound("shrinkSound", targetref, Volume, 1.0);
				
			}

			Rumbling::Once("RestoreSizeOther", targetref, 0.6, 0.05);

			if (!Revert(targetref, Power, Power/2.5)) { // Terminate the task once revert size is complete
				return false;
			}
			return true;
		});
	}
}

namespace Gts {
	std::string RestoreSizeOther::GetName() {
		return "RestoreSizeOther";
	}

	void RestoreSizeOther::OnStart() {
		Actor* target = GetTarget();
		Actor* caster = GetCaster();
		if (!target) {
			return;
		}
		if (!caster) {
			return;
		}
		float Volume = std::clamp(get_visual_scale(target) * 0.1f, 0.15f, 1.0f);
		Runtime::PlaySound("shrinkSound", target, Volume, 1.0);

		Task_RestoreSizeTask(caster, target, DualCasted());
	}

}
