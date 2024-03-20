#pragma once
// Module that handles AttributeAdjustment
#include "events.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class EmotionManager : public EventListener {
		public:
			[[nodiscard]] static EmotionManager& GetSingleton() noexcept;
			

			virtual std::string DebugName() override;
			
			virtual void Reset() override;

			void OverridePhenome(Actor* giant, int number, float power, float halflife, float target);
			void OverrideModifier(Actor* giant, int number, float power, float halflife, float target);
	};
}
