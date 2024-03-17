#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	void SurvivalMode_RemoveAllSpells(Actor* actor, SpellItem* stage0, SpellItem* stage1, SpellItem* stage2, SpellItem* stage3, SpellItem* stage4, SpellItem* stage5);
	void SurvivalMode_RefreshSpells(Actor* actor, float currentvalue);
	void SurvivalMode_AdjustHunger(Actor* giant, float tinyscale, float naturalsize, bool IsDragon, bool IsLiving, float type);
}
