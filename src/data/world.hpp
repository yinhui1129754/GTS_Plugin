#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;


namespace Gts {
	class World {
		public:
			static const RE::GMatrix3D& WorldToCamera();
			static const RE::NiRect<float>& ViewPort();
			static const float& WorldScale();
			static const float& WorldScaleInverse();
	};
}
