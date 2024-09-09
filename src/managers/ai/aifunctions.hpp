#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	float GetScareThreshold(Actor* giant);
	void Task_InitHavokTask(Actor* tiny);
	void KillActor(Actor* giant, Actor* tiny, bool silent);

	float GetGrowthCount(Actor* giant);
	float GetGrowthLimit(Actor* actor);
	float GetButtCrushDamage(Actor* actor);

	void ModGrowthCount(Actor* giant, float value, bool reset);
	void RecordStartButtCrushSize(Actor* giant);
	void SetButtCrushSize(Actor* giant, float value, bool reset);
	float GetButtCrushSize(Actor* giant);
	
	void ForceFlee(Actor* giant, Actor* tiny, float duration, bool apply_size_difference);
	void ScareActors(Actor* giant);
}