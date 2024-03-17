#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class HugAnimationController : public EventListener  {
		public:
			[[nodiscard]] static HugAnimationController& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			std::vector<Actor*> GetHugTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartHug(Actor* pred, Actor* prey);
			bool CanHug(Actor* pred, Actor* prey);
	};
}