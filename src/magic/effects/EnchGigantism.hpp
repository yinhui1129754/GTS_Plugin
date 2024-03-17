#pragma once
#include "magic/magic.hpp"
// Module that tracks Gigantism MGF


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Gigantism : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;

			virtual void OnUpdate() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

		private:
			float magnitude = 0.0;
	};
}
