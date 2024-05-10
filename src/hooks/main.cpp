#include "hooks/main.hpp"
#include "managers/Attributes.hpp"
#include "events.hpp"
#include "data/time.hpp"
#include "data/plugin.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "Config.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	void ExperimentalStuff() {
		if (Plugin::Live()) {
			EventDispatcher::DoBoneUpdate();
		}
	}
}

namespace Hooks
{

	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		/*REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};
		log::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x11F, 0x160), Update);
		*/// --- OLD HOOK

		REL::Relocation<uintptr_t> hook{REL::RelocationID(35565, 36564)}; // Credits to Ersh for this hook, DCA source code
		// ^ 5B2FF0, 5DACE0, main update
		log::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x748, 0xC26), Update);

		//_Update_before = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x509, 0xC26), Update_before);
		// At line 165
		// 0x1405b34c0 - 0x1405B2FF0 = 0x4D0
		// At line 170
		// 0x1405b34f9 - 0x1405B2FF0 = 0x509
		

		// if (REL::Module::IsAE()) {
		// 	auto offsetHelper = REL::IDDatabase::Offset2ID();
		// 	log::info("Dumping OFFSETS AE");
		// 	std::ostringstream dumpTxt;
		// 	dumpTxt << "Dumping OFFSETS AE";
		// 	for (auto& offsetData: offsetHelper) {
		// 		dumpTxt << '\n' << "DUMP:" << std::format("{}:{:X}:{}", offsetData.id, offsetData.offset, offsetData.offset);
		// 	}
		// 	log::info("{}", dumpTxt.str());
		// }
		/*if (REL::Module::IsSE()) {
			// auto offsetHelper = REL::IDDatabase::Offset2ID();
			// log::info("Dumping OFFSETS");
			// for (auto& offsetData: offsetHelper) {
			// 	log::info("{}:{:X}:{}", offsetData.id, offsetData.offset, offsetData.offset);
			// }
		}*/
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		
		_Update(a_this, a2);
		//log::info("Plugin Ready:{}, Plugin Live: {}", Plugin::Ready(), Plugin::Live());
		static std::atomic_bool started = std::atomic_bool(false);
		Plugin::SetOnMainThread(true);
		if (Plugin::Live()) {
			// We are not loading or in the mainmenu
			// Player loaded and not paused
			if (started.exchange(true)) {
				// Not first updated
				Time::GetSingleton().Update();
				EventDispatcher::DoUpdate();
			} else {
				// First update this load
				EventDispatcher::DoStart();
			}
		} else if (!Plugin::InGame()) {
			// Loading or in main menu
			started.store(false);
		}
		Plugin::SetOnMainThread(false);

		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			static Timer timer = Timer(5.0);
			if (timer.ShouldRun()) {
				Profilers::Report();
			}
		}
	}
}
