#pragma once

#include "events.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    class MovementForceData {
		public:
			MovementForceData();

			NiPoint3 POS_Last_Leg_L = NiPoint3(0.0, 0.0, 0.0);
			NiPoint3 POS_Last_Leg_R = NiPoint3(0.0, 0.0, 0.0);

			NiPoint3 POS_Last_Hand_L = NiPoint3(0.0, 0.0, 0.0);
			NiPoint3 POS_Last_Hand_R = NiPoint3(0.0, 0.0, 0.0);
	};

	float Get_Bone_Movement_Speed(Actor* actor, NodeMovementType type);

	class MovementForce : public EventListener {
		public:
			[[nodiscard]] static MovementForce& GetSingleton();

			virtual std::string DebugName() override;

			static MovementForceData& GetData(Actor* actor);

			std::unordered_map<FormID, MovementForceData> data;
	};
};