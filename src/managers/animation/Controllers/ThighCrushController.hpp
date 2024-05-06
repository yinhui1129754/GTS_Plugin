#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class ThighCrushController : public EventListener  {
		public:
			[[nodiscard]] static ThighCrushController& GetSingleton() noexcept;

			virtual std::string DebugName() override;
            
			std::vector<Actor*> GetThighTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartThighCrush(Actor* pred, Actor* prey);
			bool CanThighCrush(Actor* pred, Actor* prey);
	};
}