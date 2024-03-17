#include "papyrus/height.hpp"
#include "data/runtime.hpp"
#include "scale/height.hpp"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsHeight";

	// Target Scales
	void SetTargetHeight(StaticFunctionTag*, Actor* actor, float height) {
		set_target_height(actor, height);
	}

	float GetTargetHeight(StaticFunctionTag*, Actor* actor) {
		return get_target_height(actor);
	}

	void ModTargetHeight(StaticFunctionTag*, Actor* actor, float amt) {
		mod_target_height(actor, amt);
	}

	void SetMaxHeight(StaticFunctionTag*, Actor* actor, float height) {
		set_max_height(actor, height);
	}

	float GetMaxHeight(StaticFunctionTag*, Actor* actor) {
		return get_max_height(actor);
	}

	void ModMaxHeight(StaticFunctionTag*, Actor* actor, float amt) {
		mod_max_height(actor, amt);
	}

	float GetVisualHeight(StaticFunctionTag*, Actor* actor) {
		return get_visual_height(actor);
	}

	void ModTeammateHeight(StaticFunctionTag*, float amt) {
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			if (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction")) {
				mod_target_height(actor, amt);
			}
		}
	}
}

namespace Gts {
	bool register_papyrus_height(IVirtualMachine* vm) {
		vm->RegisterFunction("SetTargetHeight", PapyrusClass, SetTargetHeight);
		vm->RegisterFunction("GetTargetHeight", PapyrusClass, GetTargetHeight);
		vm->RegisterFunction("ModTargetHeight", PapyrusClass, ModTargetHeight);

		vm->RegisterFunction("SetMaxHeight", PapyrusClass, SetMaxHeight);
		vm->RegisterFunction("GetMaxHeight", PapyrusClass, GetMaxHeight);
		vm->RegisterFunction("ModMaxHeight", PapyrusClass, ModMaxHeight);

		vm->RegisterFunction("GetVisualHeight", PapyrusClass, GetVisualHeight);

		vm->RegisterFunction("ModTeammateHeight", PapyrusClass, ModTeammateHeight);

		return true;
	}
}
