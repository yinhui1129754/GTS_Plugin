#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class RandomGrowth : public EventListener {
		public:
			[[nodiscard]] static RandomGrowth& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
	};
}
