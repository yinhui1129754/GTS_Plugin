#include "data/world.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;


namespace {
	inline static RE::GMatrix3D* RawWorldToCamMatrix = (RE::GMatrix3D*) REL::RelocationID(519579, 406126).address();
	inline static RE::NiRect<float>* RawViewPort = (RE::NiRect<float>*) REL::RelocationID(519618,406160).address();
	inline static float* RawWorldScale = (float*)RELOCATION_ID(231896, 188105).address();
	inline static float* RawWorldScaleInverse = (float*)RELOCATION_ID(230692, 187407).address();
}

namespace Gts {
	const RE::GMatrix3D& World::WorldToCamera() {
		return *RawWorldToCamMatrix;
	}
	const RE::NiRect<float>& World::ViewPort() {
		return *RawViewPort;
	}
	const float& World::WorldScale() {
		return *RawWorldScale;
	}
	const float& World::WorldScaleInverse() {
		return *RawWorldScaleInverse;
	}
}
