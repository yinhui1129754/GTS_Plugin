#pragma once
#include "magic/magic.hpp"
#include "timer.hpp"
// Module that handles slow growth


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SlowGrow : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			virtual void OnStart() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;
		private:
			bool IsDual = false;
			Timer timer = Timer(2.33); // Run every 2.33s or as soon as we can
			Timer MoanTimer = Timer(4.66);
	};
}
