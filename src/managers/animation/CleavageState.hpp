#pragma once

#include "managers/animation/Utils/CooldownManager.hpp"
#include "events.hpp"


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{

	class Animation_Cleavage {
		public:
			static void LaunchCooldownFor(Actor* giant, CooldownSource Source);
			static void RegisterEvents();
			static void RegisterTriggers();
	};
}
