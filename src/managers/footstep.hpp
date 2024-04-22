#pragma once
// Module that handles footsteps

#include "managers/impact.hpp"
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	enum class SizeClass {
		x2,
		x4,
		x8,
		x12,
		x24,
		x48,
		x96,
		mega,
	};
	class FootStepManager : public EventListener {
		public:
			[[nodiscard]] static FootStepManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void OnImpact(const Impact& impact) override;
			static void PlayLegacySounds(NiAVObject* foot, FootEvent foot_kind, float scale, float start_l, float start_xl, float start_xxl);
			static void PlayHighHeelSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting);
			static void PlayNormalSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting);
	};
}
