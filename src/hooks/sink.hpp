#pragma once
/*
 *  Hooks for sinking
 */

#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Sinking
	{
		public:
			static void Hook(Trampoline& trampoline);
	};
}
