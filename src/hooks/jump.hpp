#pragma once
/*
 *  Hooks for jumping
 */

#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Jumping
	{
		public:
			static void Hook(Trampoline& trampoline);
		//private:
			//static float GetScaleJumpHook(TESObjectREFR* a_this);
			//static inline REL::Relocation<decltype(GetScaleJumpHook)> _GetScaleJumpHook;
	};
}
