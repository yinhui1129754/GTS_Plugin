#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Projectiles
	{
		public:
			static void Hook();
		private:
			static void GetLinearVelocityProjectile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
			static void Handle3DLoaded_Arrow(RE::Projectile* a_this);
			static void AddImpact_Arrow(RE::Projectile* a_this, TESObjectREFR* a_ref, const NiPoint3& a_targetLoc, const NiPoint3& a_velocity, hkpCollidable* a_collidable, std::int32_t a_arg6, std::uint32_t a_arg7);
			static void Handle3DLoaded_Missile(RE::Projectile* a_this);
			static void Handle3DLoaded_Beam(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
			static void Handle3DLoaded_Barrier(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
			static void Handle3DLoaded_Cone(RE::Projectile* a_this);
			static void Handle3DLoaded_Flame(RE::Projectile* a_this);
			static void Initialize_Explosion(RE::Explosion* a_this, TESObjectCELL& a_cell);

			static inline REL::Relocation<decltype(GetLinearVelocityProjectile)> _GetLinearVelocityProjectile;
			static inline REL::Relocation<decltype(Handle3DLoaded_Arrow)> _Handle3DLoaded_Arrow;
			static inline REL::Relocation<decltype(AddImpact_Arrow)> _AddImpact_Arrow;
			static inline REL::Relocation<decltype(Handle3DLoaded_Missile)> _Handle3DLoaded_Missile;
			static inline REL::Relocation<decltype(Handle3DLoaded_Beam)> _Handle3DLoaded_Beam;
			static inline REL::Relocation<decltype(Handle3DLoaded_Barrier)> _Handle3DLoaded_Barrier;
			static inline REL::Relocation<decltype(Handle3DLoaded_Cone)> _Handle3DLoaded_Cone;
			static inline REL::Relocation<decltype(Handle3DLoaded_Flame)> _Handle3DLoaded_Flame;
			static inline REL::Relocation<decltype(Initialize_Explosion)> _Initialize_Explosion;
	};
}