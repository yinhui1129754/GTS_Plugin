#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_PlayerCamera
	{
		public:
			static void Hook();
		private:
			static void Update(PlayerCamera* a_this);
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}
