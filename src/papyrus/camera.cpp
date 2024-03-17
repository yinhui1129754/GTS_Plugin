#include "papyrus/camera.hpp"
#include "data/persistent.hpp"
#include "managers/cameras/camutil.hpp"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsCamera";

	void SetEnableCollisionActor(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().camera_collisions.enable_actor = enabled;
	}

	bool GetEnableCollisionActor(StaticFunctionTag*) {
		return Persistent::GetSingleton().camera_collisions.enable_actor;
	}

	void SetEnableCollisionTree(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().camera_collisions.enable_trees = enabled;
	}

	bool GetEnableCollisionTree(StaticFunctionTag*) {
		return Persistent::GetSingleton().camera_collisions.enable_trees;
	}

	void SetEnableCollisionDebris(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().camera_collisions.enable_debris = enabled;
	}

	bool GetEnableCollisionDebris(StaticFunctionTag*) {
		return Persistent::GetSingleton().camera_collisions.enable_debris;
	}

	void SetEnableCollisionTerrain(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().camera_collisions.enable_terrain = enabled;
	}

	bool GetEnableCollisionTerrain(StaticFunctionTag*) {
		return Persistent::GetSingleton().camera_collisions.enable_terrain;
	}

	void SetEnableCollisionStatic(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().camera_collisions.enable_static = enabled;
	}

	bool GetEnableCollisionStatic(StaticFunctionTag*) {
		return Persistent::GetSingleton().camera_collisions.enable_static;
	}

	void SetCollisionScale(StaticFunctionTag*, float scale) {
		Persistent::GetSingleton().camera_collisions.above_scale = scale;
	}

	float GetCollisionScale(StaticFunctionTag*) {
		return Persistent::GetSingleton().camera_collisions.above_scale;
	}

	void ToggleFreeCamera(StaticFunctionTag*) {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			camera->ToggleFreeCameraMode(false);
		}
	}
	void ResetTheCamera(StaticFunctionTag*) {
		ResetIniSettings();
	}
}

namespace Gts {
	bool register_papyrus_camera(IVirtualMachine* vm) {
		vm->RegisterFunction("SetEnableCollisionActor", PapyrusClass, SetEnableCollisionActor);
		vm->RegisterFunction("GetEnableCollisionActor", PapyrusClass, GetEnableCollisionActor);
		vm->RegisterFunction("SetEnableCollisionTree", PapyrusClass, SetEnableCollisionTree);
		vm->RegisterFunction("GetEnableCollisionTree", PapyrusClass, GetEnableCollisionTree);
		vm->RegisterFunction("SetEnableCollisionDebris", PapyrusClass, SetEnableCollisionDebris);
		vm->RegisterFunction("GetEnableCollisionDebris", PapyrusClass, GetEnableCollisionDebris);
		vm->RegisterFunction("SetEnableCollisionTerrain", PapyrusClass, SetEnableCollisionTerrain);
		vm->RegisterFunction("GetEnableCollisionTerrain", PapyrusClass, GetEnableCollisionTerrain);
		vm->RegisterFunction("SetEnableCollisionStatic", PapyrusClass, SetEnableCollisionStatic);
		vm->RegisterFunction("GetEnableCollisionStatic", PapyrusClass, GetEnableCollisionStatic);
		vm->RegisterFunction("SetCollisionScale", PapyrusClass, SetCollisionScale);
		vm->RegisterFunction("GetCollisionScale", PapyrusClass, GetCollisionScale);
		vm->RegisterFunction("ToggleFreeCamera", PapyrusClass, ToggleFreeCamera);
		vm->RegisterFunction("ResetTheCamera", PapyrusClass, ResetTheCamera);

		return true;
	}
}
