#pragma once
/*
 *  Hooks on various Stealth calls
 */

#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Stealth
	{
		public:
			static void Hook(Trampoline& trampoline);
	};
}