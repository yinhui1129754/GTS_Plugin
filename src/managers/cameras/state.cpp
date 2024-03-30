#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/cameras/state.hpp"
#include "scale/modscale.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"



using namespace RE;
using namespace Gts;

namespace Gts {
	void CameraState::EnterState() {
	}
	void CameraState::ExitState() {
	}

	float CameraState::GetScale() {
		auto player = GetCameraActor();
		float result = get_visual_scale(player);
		if (!player) {
			return 1.0;
		}
		return result;
	}

	float CameraState::GetScaleOverride(bool IsCrawling) {
		return -1.0;
	}

	NiPoint3 CameraState::GetOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}
	NiPoint3 CameraState::GetOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetOffset(cameraPosLocal);
	}
	NiPoint3 CameraState::GetOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		if (IsCrawling) {
			return this->GetOffsetProne(cameraPosLocal);
		} else {
			return this->GetOffset(cameraPosLocal);
		}
	}

	NiPoint3 CameraState::GetCombatOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}
	NiPoint3 CameraState::GetCombatOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetCombatOffset(cameraPosLocal);
	}
	NiPoint3 CameraState::GetCombatOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		if (IsCrawling) {
			return this->GetCombatOffsetProne(cameraPosLocal);
		} else {
			return this->GetCombatOffset(cameraPosLocal);
		}
	}

	NiPoint3 CameraState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}
	NiPoint3 CameraState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetPlayerLocalOffset(cameraPosLocal);
	}
	NiPoint3 CameraState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		if (IsCrawling) {
			return this->GetPlayerLocalOffsetProne(cameraPosLocal);
		} else {
			return this->GetPlayerLocalOffset(cameraPosLocal);
		}
	}

	bool CameraState::PermitManualEdit() {
		return true;
	}
	bool CameraState::PermitTransition() {
		return true;
	}

	bool CameraState::PermitCameraTransforms() {
		return true;
	}
}
