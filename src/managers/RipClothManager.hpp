#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class ClothManager : public EventListener {
		public:
			[[nodiscard]] static ClothManager& GetSingleton() noexcept;
			virtual std::string DebugName() override;

			void CheckRip();

			float LastPlayerScale = 0.0;
			float AddedThreshold = -1.0;                        //If This Value is -1 We Know We just started
			const float TearThreshold = 1.5;
			const float TooBig = 2.5;
	};
}
