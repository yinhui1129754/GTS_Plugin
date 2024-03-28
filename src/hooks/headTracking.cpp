#include "hooks/headTracking.hpp"
#include "utils/actorUtils.hpp"
#include "scale/modscale.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;


namespace {
	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			if (IsRagdolled(giant)) { // For some reason breaks tiny ragdoll when they're small, so they fly into the sky.
				return original;      // So this check is a must because of that bug.
			}
			return original * (get_giantess_scale(giant));// / game_getactorscale(giant)); // Compensate it, since SetScale() already affects HT by default
		}
		return original;
	}
}

namespace Hooks
{

	void Hook_HeadTracking::Hook(Trampoline& trampoline) {
		static CallHook<float(TESObjectREFR* param_1)>Alter_Headtracking( 
			REL::RelocationID(37129, 37364), REL::Relocate(0x24, 0x5E),
			[](auto* param_1) {
				// ----------------- SE:
				// FUN_140615030 : 37129
				// 0x140615054 - 0x140615030 = 0x24

				//------------------ AE:
				// FUN_1405ffc50: 37364
				// 0x1405ffcae - 0x1405ffc50 = 0x5E
  
				float result = Alter_Headtracking(param_1);
				float Alter = affect_by_scale(param_1, result);
				//log::info("(20) Alter_Headtracking Hooked");
				return Alter;
            }
        );

		/*static CallHook<float(TESObjectREFR* param_1)>GetEyeHeight_140601E40(  // Get Eye Height, rarely called
			REL::RelocationID(36845, 37869), REL::Relocate(0x71, 0x71),
			[](auto* param_1) {
				// 36845
				// 0x140601eb1 - 0x140601E40 = 0x71

				//AE: (99% correct, seems to match the function)
				// FUN_140629d00 (48 83 ec 58)
				// 0x140629d71 - 0x140629d00 = 0x71
				float result = GetEyeHeight_140601E40(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(23) GetEyeHeight_140601E40 Hooked");
				return Alter;
            }
        );*/
	}
}

