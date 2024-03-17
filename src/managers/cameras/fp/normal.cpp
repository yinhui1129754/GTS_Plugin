#include "managers/cameras/fp/normal.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "scale/height.hpp"

using namespace RE;

namespace Gts {
	float FirstPerson::GetScaleOverride(bool IsCrawling) {
		float proneFactor = 1.005;
		if (IsCrawling) {
			return proneFactor; // 1.0 only if we crawl
		} else {
			return -1.0;
		}
	}
}
