#include "hooks/Projectiles.hpp"
#include "utils/camera.hpp"
#include "scale/scale.hpp"



using namespace RE;

namespace {
	void ScaleExplosion(Explosion* explosion) {
		auto causer = explosion->GetExplosionRuntimeData().actorOwner;
		if (causer) {
			auto cause = causer.get().get();
			if (cause) {
				if (IsDragon(cause) || IsGiant(cause)) {
					log::info("Scaling Explosion");
					explosion->GetExplosionRuntimeData().radius *= get_visual_scale(cause);
				}
			}
		}
	}
	void ArrowImpact(Projectile* projectile) {
		auto owner = projectile->GetProjectileRuntimeData().shooter;
		auto node = projectile->Get3D2();
		if (node) {
			if (owner) {
				auto owner_get = skyrim_cast<Actor*>(owner.get().get());
				if (owner_get) {
					auto player = PlayerCharacter::GetSingleton();
					float distance = player->GetPosition().Length();
					//log::info("Owner_get found: {}", owner_get->GetDisplayFullName());
					float scaling = get_visual_scale(owner_get);
					float shake_power = scaling * (node->world.translate.Length() / distance);

					shake_camera_at_node(node->world.translate, shake_power, 1.5);

					auto cell = projectile->GetParentCell();

					if (cell) {
						// Crashes if called like that, too lazy to fix it
						log::info("Spawning footstep.nif");
						BSTempEffectParticle::Spawn(cell, 6.0, "GTS/Effects/Footstep.nif", node->world.rotate, node->world.translate, scaling, 7, nullptr);
					}
				}
			}
		}
	}
	void ScaleProjectile(Projectile* projectile, float speed_limit, bool apply_speed) {
		auto node = projectile->Get3D2();
		if (!node) {
			node = projectile->Get3D1(false);
			if (!node) {
				log::info("3d1: fp");
				node = projectile->Get3D1(true);
			}
		} if (!node) {
			node = projectile->Get3D();
		} 
		if (node) {
			//log::info("Node found");
			auto owner = projectile->GetProjectileRuntimeData().shooter;
			if (owner) {
				//log::info("Owner found");
				auto owner_get = skyrim_cast<Actor*>(owner.get().get());
				if (owner_get) {
					//log::info("Owner_get found: {}", owner_get->GetDisplayFullName());
					float scaling = std::clamp(get_visual_scale(owner_get), 0.02f, 1.0f); // not bigger than 1.0x
					node->local.scale *= scaling;

					if (apply_speed) {
						float speed_scaling = std::clamp(scaling, 0.10f, speed_limit);
						projectile->GetProjectileRuntimeData().speedMult *= speed_scaling;
						
						//projectile->GetProjectileRuntimeData().power *= speed_scaling; // No idea what it does
					}

					auto spell = projectile->GetProjectileRuntimeData().spell;
					projectile->GetProjectileRuntimeData().scale *= scaling;
					if (spell) {
						auto effect = skyrim_cast<SpellItem*>(spell);
						if (effect) {
							log::info("Effect found!");
							effect->data.range *= scaling;
						}
					}
					auto explosion = projectile->GetProjectileRuntimeData().explosion;
					if (explosion) {
						//explosion->data.radius *= scaling;
						// Scales only visuals. Damage zone is still the same
					}
				}
			}
		}
	}
}

namespace Hooks
{
	void Hook_Projectiles::Hook() {
		///
		///  IT IS DISABLED INSIDE HOOKS CPP!
		///
        REL::Relocation<std::uintptr_t> ProjectileVtbl{ RE::VTABLE_Projectile[0] };				// 167C888
		REL::Relocation<std::uintptr_t> ArrowProjectileVtbl{ RE::VTABLE_ArrowProjectile[0] };			// 1676318
		REL::Relocation<std::uintptr_t> MissileProjectileVtbl{ RE::VTABLE_MissileProjectile[0] };		// 167AE78
		REL::Relocation<std::uintptr_t> BeamProjectileVtbl{ RE::VTABLE_BeamProjectile[0] };          // 1677660
		REL::Relocation<std::uintptr_t> BarrierProjectileVtbl{ RE::VTABLE_BarrierProjectile[0] };
		REL::Relocation<std::uintptr_t> ConeProjectileVtbl{ RE::VTABLE_ConeProjectile[0] };
		REL::Relocation<std::uintptr_t> FlameProjectileVtbl{ RE::VTABLE_FlameProjectile[0] }; 
		REL::Relocation<std::uintptr_t> ExplosionVtbl{ RE::VTABLE_Explosion[0] };

		//_GetLinearVelocityProjectile = ProjectileVtbl.write_vfunc(0x86, GetLinearVelocityProjectile); // We don't need this one
		_Handle3DLoaded_Arrow = ArrowProjectileVtbl.write_vfunc(0xC0, Handle3DLoaded_Arrow);
		/*_AddImpact_Arrow = ArrowProjectileVtbl.write_vfunc(0xBD, AddImpact_Arrow);
		_Handle3DLoaded_Missile = MissileProjectileVtbl.write_vfunc(0xC0, Handle3DLoaded_Missile);
		_Handle3DLoaded_Beam = BeamProjectileVtbl.write_vfunc(0xC0, Handle3DLoaded_Beam);
		_Handle3DLoaded_Barrier = BarrierProjectileVtbl.write_vfunc(0xC0, Handle3DLoaded_Barrier);
		_Handle3DLoaded_Cone = ConeProjectileVtbl.write_vfunc(0xC0, Handle3DLoaded_Cone);
		_Handle3DLoaded_Flame = FlameProjectileVtbl.write_vfunc(0xC0, Handle3DLoaded_Flame); // FlameProjectile.cpp -> Handle3DLoaded (C0)

		_Initialize_Explosion = ExplosionVtbl.write_vfunc(0x90, Initialize_Explosion);*/
	}

	void Hook_Projectiles::GetLinearVelocityProjectile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
	{
		// Unused
		log::info("Projectile True");
		_GetLinearVelocityProjectile(a_this, a_outVelocity);
		ScaleProjectile(a_this, 10000.0, true);
	}

	void Hook_Projectiles::Handle3DLoaded_Arrow(RE::Projectile* a_this)
	{
		// Scale Arrow projectile once (when it first spawns). Affects only visuals.
		//log::info("Arrow True");
		_Handle3DLoaded_Arrow(a_this);
		ScaleProjectile(a_this, 1.0, false);
	}

	void Hook_Projectiles::AddImpact_Arrow(RE::Projectile* a_this, TESObjectREFR* a_ref, const NiPoint3& a_targetLoc, const NiPoint3& a_velocity, hkpCollidable* a_collidable, std::int32_t a_arg6, std::uint32_t a_arg7) {
		_AddImpact_Arrow(a_this, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
		log::info("Impact Called!");
		
		if (a_ref) {
			log::info("REF: {}", a_ref->GetDisplayFullName());
			//ArrowImpact(a_this); // Better not call it, spawning Footstep.nif loves to ctd the game
		}
	}

	void Hook_Projectiles::Handle3DLoaded_Missile(RE::Projectile* a_this) {	
		// Scale Missile-like projectiles once (when they first spawn). Affects only visuals.
		log::info("Missile True");
		_Handle3DLoaded_Missile(a_this);
		ScaleProjectile(a_this, 10000.0, true);
	}

	void Hook_Projectiles::Handle3DLoaded_Beam(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity) {	
		// Scale Beam-like projectiles once (when they first spawn). Affects only visuals.
		log::info("Beam True");
		_Handle3DLoaded_Beam(a_this, a_outVelocity);
		ScaleProjectile(a_this, 10000.0, true);
	}

	void Hook_Projectiles::Handle3DLoaded_Barrier(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity) {
		// Scale Barrier-like projectiles once (when they first spawn). Affects only visuals.
		log::info("Barrier True");
		_Handle3DLoaded_Barrier(a_this, a_outVelocity);
		ScaleProjectile(a_this, 10000.0, true);
	}

	void Hook_Projectiles::Handle3DLoaded_Cone(RE::Projectile* a_this) {
		// Scale Cone-like projectiles once (when they first spawn). Affects only visuals.
		log::info("Cone True: {}", a_this->GetDisplayFullName());
		_Handle3DLoaded_Cone(a_this);
		ScaleProjectile(a_this, 10000.0, true);
	}

	void Hook_Projectiles::Handle3DLoaded_Flame(RE::Projectile* a_this) {
		// Scale Flame-like projectiles once (when they first spawn). Affects only visuals.
		log::info("Flame True: {}", a_this->GetDisplayFullName());
		_Handle3DLoaded_Flame(a_this);
		ScaleProjectile(a_this, 10000.0, false);
	}

	void Hook_Projectiles::Initialize_Explosion(RE::Explosion* a_this, TESObjectCELL& a_cell) {
		// Attempts to scale explosions that come from dragons. Doesn't work s expected.
		log::info("Explosion True");
		_Initialize_Explosion(a_this, a_cell);
		ScaleExplosion(a_this);
	}
}