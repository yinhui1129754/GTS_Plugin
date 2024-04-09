#pragma once
#include "magic/magic.hpp"
#include "timer.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class RestoreSizeOther : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;

			virtual std::string GetName() override;
	};
}
