#pragma once
#include "managers/cameras/state.hpp"

using namespace RE;

namespace Gts {
	class FirstPersonCameraState : public CameraState {
		public:
			virtual void ExitState() override;

			virtual float GetScale() override;

			virtual bool PermitManualEdit() override;
			virtual bool PermitTransition() override;
			virtual bool PermitCameraTransforms() override;
	};
}
