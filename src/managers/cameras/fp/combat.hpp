#pragma once
#include "managers/cameras/fpState.hpp"

using namespace RE;

namespace Gts {
	class FirstPersonCombat : public FirstPersonCameraState {
		public:
			virtual float GetScaleOverride(bool IsCrawling) override;
	};
}
