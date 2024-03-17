#include "managers/cameras/trans.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	TransState::TransState(CameraState* stateA, CameraState* stateB) : stateA(stateA), stateB(stateB) {
		this->smoothIn.value = 0.0;
		this->smoothIn.target = 1.0;
		this->smoothIn.velocity = 0.0;
	}
	float TransState::GetScale() {
		return this->stateB->GetScale() * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetScale() * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetOffset(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetOffset(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetOffset(cameraPosLocal) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}
	NiPoint3 TransState::GetOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		return this->stateB->GetOffset(cameraPosLocal, IsCrawling) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetOffset(cameraPosLocal, IsCrawling) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}
	NiPoint3 TransState::GetOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetOffsetProne(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetOffsetProne(cameraPosLocal) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetCombatOffset(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetCombatOffset(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetCombatOffset(cameraPosLocal) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}
	NiPoint3 TransState::GetCombatOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		return this->stateB->GetCombatOffset(cameraPosLocal, IsCrawling) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetCombatOffset(cameraPosLocal, IsCrawling) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}
	NiPoint3 TransState::GetCombatOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetCombatOffsetProne(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetCombatOffsetProne(cameraPosLocal) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetPlayerLocalOffset(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetPlayerLocalOffset(cameraPosLocal) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}
	NiPoint3 TransState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		return this->stateB->GetPlayerLocalOffset(cameraPosLocal, IsCrawling) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetPlayerLocalOffset(cameraPosLocal, IsCrawling) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}
	NiPoint3 TransState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetPlayerLocalOffsetProne(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetPlayerLocalOffsetProne(cameraPosLocal) * (1.0 - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	bool TransState::PermitManualEdit() {
		return this->stateB->PermitManualEdit() && this->stateA->PermitManualEdit();
	}

	bool TransState::PermitTransition() {
		return false;
	}
	bool TransState::PermitCameraTransforms() {
		return this->stateB->PermitCameraTransforms() && this->stateA->PermitCameraTransforms();
	}

	bool TransState::IsDone() {
		return std::clamp(this->smoothIn.value, 0.0f, 1.0f) > 0.995;
	}
}
