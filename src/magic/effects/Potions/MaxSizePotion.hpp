#pragma once
#include "magic/magic.hpp"
// Module that handles Size Potion


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class MaxSizePotion : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

			MaxSizePotion(ActiveEffect* effect);
		private:
			float Power = 0.0;
	};
}
