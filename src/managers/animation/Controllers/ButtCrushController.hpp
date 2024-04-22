#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class ButtCrushController : public EventListener  {
		public:
			[[nodiscard]] static ButtCrushController& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void ButtCrush_OnCooldownMessage(Actor* giant);
			std::vector<Actor*> GetButtCrushTargets(Actor* pred, std::size_t numberOfPrey);
			static void StartButtCrush(Actor* pred, Actor* prey);
			bool CanButtCrush(Actor* pred, Actor* prey);
	};
}