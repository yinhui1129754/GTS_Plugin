#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace RE;

namespace Gts {
	enum class ChosenGameMode {
		None = 0,
		Grow = 1,
		Shrink = 2,
		Standard = 3,
		StandardNoShrink = 4,
		CurseOfGrowth = 5,
		Quest = 6,
	};

	class GameModeManager : public EventListener  {
		public:
			[[nodiscard]] static GameModeManager& GetSingleton() noexcept;
			virtual std::string DebugName() override;

			void ApplyGameMode(Actor* actor, const ChosenGameMode& game_mode, const float& GrowthRate, const float& ShrinkRate);
			void GameMode(Actor* actor);
	};
}
