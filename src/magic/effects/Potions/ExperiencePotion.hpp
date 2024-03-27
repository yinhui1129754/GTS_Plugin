#pragma once
#include "magic/magic.hpp"
// Module that handles Size Potion


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ExperiencePotion : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;
	};
}
