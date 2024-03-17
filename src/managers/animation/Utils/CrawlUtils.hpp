#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {

	void DoCrawlingSounds(Actor* actor, float scale, NiAVObject* node, FootEvent foot_kind);

	void DoCrawlingFunctions(Actor* actor, float scale, float multiplier, float damage, CrawlEvent kind, std::string_view tag, float launch_dist, float damage_dist, float crushmult, DamageSource Cause);

	void DoDamageAtPoint(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, DamageSource Cause);
	
	void ApplyAllCrawlingDamage(Actor* giant, int random, float bonedamage);
}