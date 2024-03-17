#pragma once
#include "managers/cameras/tpState.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class Foot : public ThirdPersonCameraState {
		public:
			virtual void EnterState() override;

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPos) override;
		protected:
			virtual NiPoint3 GetFootPos();

			Spring3 smoothFootPos = Spring3(NiPoint3(0.0, 0.0, 0.0), 0.5);

			Spring smoothScale = Spring(1.0, 0.5);
	};
}
