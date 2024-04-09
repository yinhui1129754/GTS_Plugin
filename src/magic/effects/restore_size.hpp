#pragma once
#include "magic/magic.hpp"
#include "timer.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class RestoreSize : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;

			virtual std::string GetName() override;

		private:
			Timer timer = Timer(2.33); // Run every 2.33s or as soon as we can
	};
}
