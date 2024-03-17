#pragma once
// Animation: Compatibility
// Notes: Made avaliable for other generic anim mods
//  - Stages
//    - "GTScrush_caster",          //[0] The gainer.
//    - "GTScrush_victim",          //[1] The one to crush
// Notes: Modern Combat Overhaul compatibility
// - Stages
//   - "MCO_SecondDodge",           // enables GTS sounds and footstep effects
//   - "SoundPlay.MCO_DodgeSound",

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class AnimationCompat {
		public:
			static void RegisterEvents();
			static void RegisterTriggers();
	};
}
