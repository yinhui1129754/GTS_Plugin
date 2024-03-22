
#pragma once
/*
 *  Hooks for actor rotation
 */

#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_PreventAnimations
	{
		public:
			static void Hook(Trampoline& trampoline);
	};
}