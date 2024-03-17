#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class GrabAnimationController : public EventListener  {
		public:
			[[nodiscard]] static GrabAnimationController& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			std::vector<Actor*> GetGrabTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartGrab(Actor* pred, Actor* prey);
			bool CanGrab(Actor* pred, Actor* prey);
	};
}