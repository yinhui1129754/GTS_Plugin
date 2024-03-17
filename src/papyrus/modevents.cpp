#include "papyrus/modevents.hpp"
#include "managers/modevent.hpp"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsEvent";

	void RegisterOnFootstep(StaticFunctionTag*, TESForm* form) {
		if (!form) {
			return;
		}
		auto event_manager = ModEventManager::GetSingleton();
		event_manager.m_onfootstep.Register(form);
	}
	void UnRegisterOnFootstep(StaticFunctionTag*, TESForm* form) {
		if (!form) {
			return;
		}
		auto event_manager = ModEventManager::GetSingleton();
		event_manager.m_onfootstep.Unregister(form);
	}
}

namespace Gts {
	bool register_papyrus_events(IVirtualMachine* vm) {
		vm->RegisterFunction("RegisterOnFootstep", PapyrusClass, RegisterOnFootstep);
		vm->RegisterFunction("UnRegisterOnFootstep", PapyrusClass, UnRegisterOnFootstep);

		return true;
	}
}
