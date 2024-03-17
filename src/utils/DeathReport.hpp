#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	std::string_view GetDeathNodeName(DamageSource cause);
	void ReportDeath(Actor* giant, Actor* tiny, DamageSource cause);
}