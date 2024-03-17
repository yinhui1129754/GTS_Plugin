#include "utils/camera.hpp"
#include "utils/papyrusUtils.hpp"

#include "node.hpp"

using namespace RE;
using namespace SKSE;

namespace Gts {

	void shake_camera_script(TESObjectREFR* actor, float intensity, float duration) { // TESObjectREFR*
		CallFunction("Game", "ShakeCamera", actor, intensity, duration);
	}

	void shake_camera(TESObjectREFR* actor, float intensity, float duration) { // TESObjectREFR*
		//CallFunction("Game", "ShakeCamera", actor, intensity, duration);
		NiPoint3 position = actor->GetPosition();
		ShakeCamera(intensity, position, duration);
	}

	void shake_camera_at_node(NiPoint3 position, float intensity, float duration) { // TESObjectREFR*
		ShakeCamera(intensity, position, duration);
	}

	void shake_camera_at_node(Actor* giant, std::string_view node, float intensity, float duration) { // TESObjectREFR*
		auto bone = find_node(giant, node);
		if (bone) {
			NiPoint3 position = bone->world.translate;
			ShakeCamera(intensity, position, duration);
		}
	}

	void TriggerScreenBlood(int aiValue) {
		CallFunction("Game", "TriggerScreenBlood", aiValue);
	}

	void shake_controller(float left_intensity, float right_intensity, float duration) {
		CallFunction("Game", "ShakeController", left_intensity, right_intensity, duration);
	}

	float get_distance_to_camera(const NiPoint3& point) {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto point_a = point;
			auto point_b = camera->pos;
			auto delta = point_a - point_b;
			return delta.Length();
		}
		return 3.4028237E38; // Max float
	}

	float get_distance_to_camera(NiAVObject* node) {
		if (node) {
			return get_distance_to_camera(node->world.translate);
		}
		return 3.4028237E38; // Max float
	}

	float get_distance_to_camera(Actor* actor) {
		if (actor) {
			return get_distance_to_camera(actor->GetPosition());
		}
		return 3.4028237E38; // Max float
	}

	bool IsFirstPerson() {
		auto playercamera = PlayerCamera::GetSingleton();
		if (!playercamera) {
			return false;
		}
		if (playercamera->currentState == playercamera->cameraStates[CameraState::kFirstPerson]) {
			return true;
		}
		return false;
	}

	bool IsFreeCamera() {
		auto playercamera = PlayerCamera::GetSingleton();
		if (!playercamera) {
			return false;
		}
		if (playercamera->currentState == playercamera->cameraStates[CameraState::kFree]) {
			return true;
		}
		return false;
	}

}
