#include "managers/modevent.hpp"

using namespace SKSE;
using namespace RE;

namespace Gts {
	ModEventManager& ModEventManager::GetSingleton() noexcept {
		static ModEventManager instance;

		// static std::atomic_bool initialized;
		// static std::latch latch(1);
		// if (!initialized.exchange(true)) {
		// 	instance.m_onfootstep = OnFootstep("OnFootstep"sv);
		// 	latch.count_down();
		// }
		// latch.wait();

		return instance;
	}

	ModEventManager::ModEventManager() : m_onfootstep("OnFootstep"sv) {
	}
}
