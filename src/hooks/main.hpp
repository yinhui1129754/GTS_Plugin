#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_MainUpdate
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			static void Update(RE::Main* a_this, float a2);
			static inline REL::Relocation<decltype(Update)> _Update;

			static void PopulateHook1(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon);
			static inline REL::Relocation<decltype(PopulateHook1)> _PopulateHook1;

			static void PopulateHook2(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon);
			static inline REL::Relocation<decltype(PopulateHook2)> _PopulateHook2;

			static void PopulateHook3(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon);
			static inline REL::Relocation<decltype(PopulateHook3)> _PopulateHook3;

			static void PopulateHook4(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon);
			static inline REL::Relocation<decltype(PopulateHook1)> _PopulateHook4;
	};
}
