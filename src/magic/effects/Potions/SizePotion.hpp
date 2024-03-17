#pragma once
#include "magic/magic.hpp"
// Module that handles Size Potion


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SizePotion : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;

			virtual void OnUpdate() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

			SizePotion(ActiveEffect* effect);
		private:
			float Strenght = 0.0;
	};
}
