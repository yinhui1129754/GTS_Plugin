#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class HitManager : public EventListener
	{
		public:
			[[nodiscard]] static HitManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			void HitEvent(const TESHitEvent* evt) override;
			void Update() override;
		private:
			bool CanGrow = false;
			bool Balance_CanShrink = false;
			bool BlockEffect = false;
			inline static float BonusPower = 1.0;
			inline static float GrowthTick = 0.0;
			inline static float AdjustValue = 1.0;
	};
}
