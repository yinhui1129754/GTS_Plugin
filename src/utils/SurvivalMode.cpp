#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/SurvivalMode.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "spring.hpp"
#include "scale/scale.hpp"
#include "colliders/RE.hpp"
#include "colliders/actor.hpp"
#include "timer.hpp"
#include "node.hpp"
#include "utils/av.hpp"
#include "colliders/RE.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {

	void SurvivalMode_RemoveAllSpells(Actor* actor, SpellItem* stage0, SpellItem* stage1, SpellItem* stage2, SpellItem* stage3, SpellItem* stage4, SpellItem* stage5) {
		actor->RemoveSpell(stage0);
		actor->RemoveSpell(stage1);
		actor->RemoveSpell(stage2);
		actor->RemoveSpell(stage3);
		actor->RemoveSpell(stage4);
		actor->RemoveSpell(stage5);
	}

	void SurvivalMode_RefreshSpells(Actor* actor, float currentvalue) {
		auto stage0 = Runtime::GetSpell("Survival_HungerStage0");
		auto stage1 = Runtime::GetSpell("Survival_HungerStage1");
		auto stage2 = Runtime::GetSpell("Survival_HungerStage2");
		auto stage3 = Runtime::GetSpell("Survival_HungerStage3");
		auto stage4 = Runtime::GetSpell("Survival_HungerStage4");
		auto stage5 = Runtime::GetSpell("Survival_HungerStage5");

		auto stage1threshold = Runtime::GetFloat("Survival_HungerStage1Value");
		auto stage2threshold = Runtime::GetFloat("Survival_HungerStage2Value");
		auto stage3threshold = Runtime::GetFloat("Survival_HungerStage3Value");
		auto stage4threshold = Runtime::GetFloat("Survival_HungerStage4Value");
		auto stage5threshold = Runtime::GetFloat("Survival_HungerStage5Value");

		SurvivalMode_RemoveAllSpells(actor, stage0, stage1, stage2, stage3, stage4, stage5);

		if (currentvalue <= stage1threshold) {
			Runtime::AddSpell(actor, "Survival_HungerStage1");
		} else if (currentvalue <= stage2threshold) {
			Runtime::AddSpell(actor, "Survival_HungerStage2");
		} else if (currentvalue <= stage3threshold) {
			Runtime::AddSpell(actor, "Survival_HungerStage3");
		} else if (currentvalue <= stage4threshold) {
			Runtime::AddSpell(actor, "Survival_HungerStage4");
		} else if (currentvalue <= stage5threshold) {
			Runtime::AddSpell(actor, "Survival_HungerStage5");
		}
	}

	void SurvivalMode_AdjustHunger(Actor* giant, float tinyscale, float naturalsize, bool IsDragon, bool IsLiving, float type) {
		if (giant->formID != 0x14) {
			return; //Only for Player
		}
		auto Survival = Runtime::GetGlobal("Survival_ModeEnabled");
		if (!Survival) {
			return; // Abort if it doesn't exist (Should fix issues if we don't have AE CC mods)
		}
		float SurvivalEnabled = Runtime::GetBool("Survival_ModeEnabled");
		if (!SurvivalEnabled) {
			return; // Survival OFF, do nothing.
		}
		auto HungerNeed = Runtime::GetGlobal("Survival_HungerNeedValue"); // Obtain
		float restore = 16.0 * 6.0; // * 6.0 to compensate default size difference threshold for Vore

		if (IsDragon) {
			naturalsize *= 8.0; // Dragons are huge, makes sense to give a lot of value
		}
		if (!IsLiving) {
			naturalsize *= 0.20; // Less effective on non living targets
		}
		if (type >= 1.0) {
			restore *= 6.0; // Stronger gain on Vore Finish
		}

		float power = (get_visual_scale(giant)/tinyscale); // Get size difference and * it by natural size

		HungerNeed->value -= (restore / power) * naturalsize;
		if (HungerNeed->value <= 0.0) {
			HungerNeed->value = 0.0; // Cap it at 0.0
		}
		float value = HungerNeed->value;
		SurvivalMode_RefreshSpells(giant, value);
	}
}