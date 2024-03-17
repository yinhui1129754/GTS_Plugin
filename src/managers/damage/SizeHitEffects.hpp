#pragma once
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
// Module for accurate size-related damage

namespace Gts {
	class SizeHitEffects : public EventListener  {
		public:
			[[nodiscard]] static SizeHitEffects& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			void ApplyEverything(Actor* attacker, Actor* receiver, float damage);
			void BreakBones(Actor* giant, Actor* tiny, float damage, int random);
	};
}