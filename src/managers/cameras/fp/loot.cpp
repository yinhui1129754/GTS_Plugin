#include "managers/cameras/fp/loot.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "scale/height.hpp"

using namespace RE;

namespace {
	const float BASE_OVERRIDE = 0.7;
}

namespace Gts {
	float FirstPersonLoot::GetScaleOverride(bool IsCrawling) {
		float proneFactor = 1.0;
		if (IsCrawling) {
			proneFactor = GetProneAdjustment();
		}
		return BASE_OVERRIDE * proneFactor;
	}
}
