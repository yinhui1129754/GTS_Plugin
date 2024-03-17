#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_Havok
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			enum class CollisionFilterComparisonResult : uint8_t
			{
				Continue,  // Do not affect whether the two objects should collide
				Collide,   // Force the two objects to collide
				Ignore,    // Force the two objects to not collide
			};

			static CollisionFilterComparisonResult CompareFilterInfo(bhkCollisionFilter* a_collisionFilter, uint32_t a_filterInfoA, uint32_t a_filterInfoB);

			static void ProcessHavokHitJobs(void* a1);
			static inline REL::Relocation<decltype(ProcessHavokHitJobs)> _ProcessHavokHitJobs;

			static bool* IsCollisionEnabled(hkpCollidableCollidableFilter* a_this, bool* a_result, const hkpCollidable* a_collidableA, const hkpCollidable* a_collidableB);
			static inline REL::Relocation<decltype(IsCollisionEnabled)> _IsCollisionEnabled;
	};
}
