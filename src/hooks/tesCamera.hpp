#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_TESCamera
	{
		public:
			static void Hook();
		private:
			static void Update(TESCamera* a_this);
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}
