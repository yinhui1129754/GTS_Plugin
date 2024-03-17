#pragma once
// See https://github.com/ersh1/TrueDirectionalMovement/blob/b164fb5500a72f179fa2f0001e15397094f77c73/src/Hooks.h#L378-L387
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Damage
	{
		public:
			static void Hook(Trampoline& trampoline);
	};
}
