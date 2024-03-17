#include "hooks/sink.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Sinking::Hook(Trampoline& trampoline) {


		static FunctionHook<float(TESObjectREFR* a_this)> GetScaledBoundSize(
			RELOCATION_ID(36448, 37444),
			[](auto* a_this) {
			float result = GetScaledBoundSize(a_this);
			if (a_this) {
				Actor* actor = skyrim_cast<Actor*>(a_this);
				if (actor) {
					float scale = get_visual_scale(actor);
					if (scale > 1e-4) {
						result *= scale;
					}
				}
			}
			return result;
			}
			);

		// Enable this for stopping swiiming after a size
		// static FunctionHook<float(TESObjectREFR* a_this, float z_pos, TESObjectCELL* a_cell)> GetSubmergeLevel(
		// 	RELOCATION_ID(36452, 37448),
		// 	[](auto* a_this, float z_pos, auto* a_cell){
		// 		float result = GetSubmergeLevel(a_this, z_pos, a_cell);
		// 		Actor* actor = skyrim_cast<Actor*>(a_this);
		//
		// 		if (actor) {
		//       float scale = get_visual_scale(actor);
		//       if (scale > 10.0) {
		//         result = 0.0;
		//       }
		// 		}
		// 		return result;
		// });
	}
}
