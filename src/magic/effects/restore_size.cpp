
#include "magic/effects/restore_size.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string RestoreSize::GetName() {
		return "RestoreSize";
	}

	void RestoreSize::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		float Volume = clamp(0.10, 1.0, get_visual_scale(caster) * 0.1);
		Runtime::PlaySound("shrinkSound", caster, Volume, 1.0);
	}

	void RestoreSize::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float Power = 0.00065;

		if (DualCasted()) {
			Power *= 2.0;
		}

		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.10, 1.0, get_visual_scale(caster) * 0.1);
			Runtime::PlaySound("shrinkSound", caster, Volume, 1.0);
			GRumble::Once("RestoreSize", caster, 0.60, 0.05);
		}

		if (!Revert(caster, Power, Power/2.5)) { // Returns false when restore size is complete
			std::string taskname = std::format("DispelShrink_Other_{}", caster->formID);
			TaskManager::RunOnce(taskname, [=](auto& update) {
				Dispel();
			});
		}
	}
}
