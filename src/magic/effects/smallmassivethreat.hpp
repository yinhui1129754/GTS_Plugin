#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SmallMassiveThreat : public Magic {
		public:

			using Magic::Magic;

			virtual void OnStart() override;

			virtual void OnUpdate() override;

			virtual std::string GetName() override;

	};
}
