#pragma once
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ModEventManager {
		public:
			using OnFootstep = RegistrationSet<Actor*, std::string>;

			[[nodiscard]] static ModEventManager& GetSingleton() noexcept;

			OnFootstep m_onfootstep;
		private:
			ModEventManager();
	};
}
