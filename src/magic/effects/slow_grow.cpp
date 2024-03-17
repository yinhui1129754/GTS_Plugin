#include "magic/effects/slow_grow.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string SlowGrow::GetName() {
		return "SlowGrow";
	}

	void SlowGrow::OnUpdate() {
		const float BASE_POWER = 0.000025; // Default growth over time.
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto GtsSkillLevel = GetGtsSkillLevel();

		float AlterBonus = 1.0 + (GtsSkillLevel * 0.01); // Calculate bonus power. At the Alteration of 100 it becomes 200%.
		float power = BASE_POWER * AlterBonus;

		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.20, 1.0, get_visual_scale(caster)/8);
			Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0,  "NPC Pelvis [Pelv]");
		}
		if (this->MoanTimer.ShouldRun() && Runtime::GetFloat("AllowMoanSounds") == 1.0 && IsFemale(caster)) {
			float MoanVolume = clamp(0.25, 2.0, get_visual_scale(caster)/8);
			PlayMoanSound(caster, MoanVolume);
			//log::info("Attempting to play Moan Sound for: {}", caster->GetDisplayFullName());
		}
		float bonus = 1.0;
		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
			bonus = get_visual_scale(caster) * 0.25 + 0.75;
		}

		Grow(caster, 0.0, power * bonus);
		GRumble::Once("SlowGrow", caster, 0.30, 0.05);
		//log::info("Slowly Growing, actor: {}", caster->GetDisplayFullName());
	}

	void SlowGrow::OnStart() {
		if (IsDualCasting()) {
			this->IsDual = true;
		} else {
			this->IsDual = false;
		}
	}

	void SlowGrow::OnFinish() {
	}
}
