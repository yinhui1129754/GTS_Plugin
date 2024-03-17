#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	void KillActor(Actor* giant, Actor* tiny);

	float GetGrowthCount(Actor* giant);
	float GetGrowthLimit(Actor* actor);
	float GetButtCrushDamage(Actor* actor);

	void ModGrowthCount(Actor* giant, float value, bool reset);
	void SetBonusSize(Actor* giant, float value, bool reset);
	

	void ScareActors(Actor* giant);
}