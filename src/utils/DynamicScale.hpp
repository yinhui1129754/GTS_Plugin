#pragma once

#include "events.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	float GetCeilingHeight(Actor* giant);
	float GetMaxRoomScale(Actor* giant);

	class DynamicScaleData {
		public:
			DynamicScaleData();

			Spring roomHeight;
	};

	class DynamicScale : public EventListener {
		public:
			[[nodiscard]] static DynamicScale& GetSingleton();

			virtual std::string DebugName() override;

			static DynamicScaleData& GetData(Actor* actor);

			std::unordered_map<FormID, DynamicScaleData> data;
	};
}
