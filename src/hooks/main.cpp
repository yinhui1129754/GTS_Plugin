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

namespace Hooks
{

	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		/*REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};
		log::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x11F, 0x160), Update);
		*/// --- OLD HOOK

		REL::Relocation<uintptr_t> hook{REL::RelocationID(35565, 36564)}; // Credits to Ersh for this hook, DCA source code
		// ^ 5B2FF0, 5D9F50, main update
		log::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x748, 0xC26), Update);
		// ^ 5B3738, 5DAB76

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

		        REL::Relocation<uintptr_t*> hook1(REL::ID(37606), REL::Offset(0xb6));
		        log::info("Applying PopulateHook1: {:X}:{:X}", hook1.address(), *hook1.get());
		        _PopulateHook1 = trampoline.write_call<5>(hook1.address(), PopulateHook1);
		        log::info("  - Applied PopulateHook1");

		        REL::Relocation<uintptr_t*> hook2(REL::ID(37673), REL::Offset(0x1b7));
		        log::info("Applying PopulateHook2: {:X}:{:X}", hook2.address(), *hook2.get());
		        _PopulateHook2 = trampoline.write_call<5>(hook1.address(), PopulateHook2);
		        log::info("  - Applied PopulateHook2");

		        REL::Relocation<uintptr_t*> hook3(REL::ID(37674), REL::Offset(0xeb));
		        log::info("Applying PopulateHook3: {:X}:{:X}", hook3.address(), *hook3.get());
		        _PopulateHook3 = trampoline.write_call<5>(hook1.address(), PopulateHook3);
		        log::info("  - Applied PopulateHook3");

		        REL::Relocation<uintptr_t*> hook4(REL::ID(42830), REL::Offset(0x83));
		        log::info("Applying PopulateHook4: {:X}:{:X}", hook4.address(), *hook4.get());
		        _PopulateHook4 = trampoline.write_call<5>(hook1.address(), PopulateHook4);
		        log::info("  - Applied PopulateHook4");
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
