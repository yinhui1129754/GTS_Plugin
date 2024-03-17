#include "hooks/tesCamera.hpp"
#include "data/plugin.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	// BGSImpactManager
	void Hook_TESCamera::Hook() {
		log::info("Hooking TESCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_TESCamera[0] };
		_Update = Vtbl.write_vfunc(0x02, Update);
	}

	void Hook_TESCamera::Update(TESCamera* a_this) {
		log::info("Hook_TESCamera::Update");
		_Update(a_this);
		// CameraManager::GetSingleton().Update();
	}
}
