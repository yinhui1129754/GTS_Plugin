#include "managers/OverkillManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "hooks/havok.hpp"
#include "scale/scale.hpp"
#include "events.hpp"

#include "managers/contact.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	COL_LAYER GetCollisionLayer(const std::uint32_t& collisionFilterInfo) {
		return static_cast<COL_LAYER>(collisionFilterInfo & 0x7F);
	}
	COL_LAYER GetCollisionLayer(const hkpCollidable* collidable) {
		if (collidable) {
			return GetCollisionLayer(collidable->broadPhaseHandle.collisionFilterInfo);
		} else {
			return COL_LAYER::kUnidentified;
		}
	}
	COL_LAYER GetCollisionLayer(const hkpCollidable& collidable) {
		return GetCollisionLayer(&collidable);
	}

	std::uint32_t GetCollisionSystem(const std::uint32_t& collisionFilterInfo) {
		return collisionFilterInfo >> 16;
	}
	std::uint32_t GetCollisionSystem(const hkpCollidable* collidable) {
		if (collidable) {
			return GetCollisionSystem(collidable->broadPhaseHandle.collisionFilterInfo);
		} else {
			return 0;
		}
	}
	std::uint32_t GetCollisionSystem(const hkpCollidable& collidable) {
		return GetCollisionSystem(&collidable);
	}

	TESObjectREFR* GetTESObjectREFR(const hkpCollidable* collidable) {
		if (collidable) {
			auto type = collidable->broadPhaseHandle.type;
			if (static_cast<RE::hkpWorldObject::BroadPhaseType>(type) == hkpWorldObject::BroadPhaseType::kEntity) {
				if (collidable->ownerOffset < 0) {
					hkpRigidBody* obj = collidable->GetOwner<hkpRigidBody>();
					if (obj) {
						return obj->GetUserData();
					}
				}
			} else if (static_cast<RE::hkpWorldObject::BroadPhaseType>(type) == hkpWorldObject::BroadPhaseType::kPhantom) {
				if (collidable->ownerOffset < 0) {
					hkpPhantom* obj = collidable->GetOwner<hkpPhantom>();
					if (obj) {
						return obj->GetUserData();
					}
				}
			}
		}
		return nullptr;
	}
	TESObjectREFR* GetTESObjectREFR(const hkpCollidable& collidable) {
		return GetTESObjectREFR(&collidable);
	}

	bool IsCollisionDisabledBetween(TESObjectREFR* actor, TESObjectREFR* otherActor) {
		if (!actor) {
			return false;
		}
		if (!otherActor) {
			return false;
		}
		auto tranDataA = Transient::GetSingleton().GetData(actor);
		if (tranDataA) {
			if (tranDataA->disable_collision_with == otherActor) {
				return true;
			}
		}

		auto tranDataB = Transient::GetSingleton().GetData(otherActor);
		if (tranDataB) {
			if (tranDataB->disable_collision_with == actor) {
				return true;
			}
		}

		Actor* actor_a = skyrim_cast<Actor*>(actor);
		Actor* actor_b = skyrim_cast<Actor*>(otherActor);

		if (actor_a && actor_b) {

			float gts_scale = get_visual_scale(actor_a);
			float tiny_scale = get_visual_scale(actor_b);

			float Scale_A = get_visual_scale(actor_a) * GetSizeFromBoundingBox(actor_a); // A is usually GTS, but can be TIny as well
			float Scale_B = get_visual_scale(actor_b) * GetSizeFromBoundingBox(actor_b); // B is usually a tiny but can be GTS as well

			float sizedifference_gts = Scale_A/Scale_B;

			if (gts_scale/tiny_scale < 1.0) { // Switch actor roles
				//log::info("Roles switched");
				sizedifference_gts = Scale_B/Scale_A; // Rough fix for Tiny being the Gts sometimes
			}

			float limit = 3.0;

			bool ignore = (sizedifference_gts >= limit);
			if (ignore) {
				return true;
			}
		}


		return false;
	}

	void CollisionPrints(const hkpCollidable* collidableA, const hkpCollidable* collidableB) {
		
		auto ObjectA = GetTESObjectREFR(collidableA);
		auto ObjectB = GetTESObjectREFR(collidableB);

		if (ObjectA && ObjectB) {
			auto Layer_A = GetCollisionLayer(collidableA);
			auto Layer_B = GetCollisionLayer(collidableB);
			if (ObjectA->formID == 0x14 || ObjectB->formID == 0x14) {
				log::info("{} is colliding with {}", ObjectA->GetDisplayFullName(), ObjectB->GetDisplayFullName());
				log::info("{} Collision Layer: {}", ObjectA->GetDisplayFullName(), Layer_A);
				log::info("{} Collision Layer: {}", ObjectB->GetDisplayFullName(), Layer_B);
			}
		}
	}

	void Throw_DoDamage(TESObjectREFR* victim_ref, TESObjectREFR* aggressor_ref, float speed) {
		float damage = speed * Damage_Throw_Collision;

		Actor* victim = skyrim_cast<Actor*>(victim_ref);
		Actor* aggressor = skyrim_cast<Actor*>(aggressor_ref);

		if (victim && aggressor) {
			InflictSizeDamage(aggressor, victim, damage);

			std::string task = std::format("ThrowTiny {}", victim->formID);
			ActorHandle giantHandle = aggressor->CreateRefHandle();
			ActorHandle tinyHandle = victim->CreateRefHandle();

			log::info("Inflicting throw damage for {}: {}", victim->GetDisplayFullName(), damage);

			TaskManager::RunOnce(task, [=](auto& update){
				if (!giantHandle) {
					return;
				}
				if (!tinyHandle) {
					return;
				}
				
				auto giant = giantHandle.get().get();
				auto tiny = tinyHandle.get().get();
				float health = GetAV(tiny, ActorValue::kHealth);
				if (health <= 1.0 || tiny->IsDead()) {
					OverkillManager::GetSingleton().Overkill(giant, tiny);
				}
			});
		}
	}

	void Throw_ThrowCheck(TESObjectREFR* objA, TESObjectREFR* objB, COL_LAYER Layer_A, COL_LAYER Layer_B) {
		if (!objA) {
			return;
		}
		if (!objB) {
			return;
		}

		log::info("Throw check running");

		if (Layer_A == COL_LAYER::kStatic || Layer_B == COL_LAYER::kStatic) {
			
			log::info("Throw check passed");
			log::info("{} collides with {}", objA->GetDisplayFullName(), objB->GetDisplayFullName());

			auto tranDataA = Transient::GetSingleton().GetData(objA);
			if (tranDataA) {
				if (tranDataA->Throw_Offender) {
					Throw_DoDamage(objA, tranDataA->Throw_Offender, tranDataA->Throw_Speed);
					tranDataA->Throw_WasThrown = false;
					tranDataA->Throw_Offender = nullptr;
					tranDataA->Throw_Speed = 0.0;
					return;
				}
			}

			auto tranDataB = Transient::GetSingleton().GetData(objB);
			if (tranDataB) {
				if (tranDataB->Throw_Offender) {
					Throw_DoDamage(objB, tranDataB->Throw_Offender, tranDataB->Throw_Speed);
					tranDataB->Throw_WasThrown = false;
					tranDataB->Throw_Offender = nullptr;
					tranDataB->Throw_Speed = 0.0;
					return;
				}
			}
		}
	}
}

namespace Hooks
{
	void Hook_Havok::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{RELOCATION_ID(38112, 39068)}; // SE: 6403D0
		log::info("Gts applying Havok Hook at {}", hook.address());
		_ProcessHavokHitJobs = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x104, 0xFC), ProcessHavokHitJobs);

		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_bhkCollisionFilter[1] };
		_IsCollisionEnabled = Vtbl.write_vfunc(0x1, IsCollisionEnabled);
	}

	void Hook_Havok::ProcessHavokHitJobs(void* a1)
	{
		_ProcessHavokHitJobs(a1);

		EventDispatcher::DoHavokUpdate();
	}

	// Credit: FlyingParticle for code on getting the TESObjectREFR
	//         maxsu. for IsCollisionEnabled idea
	bool* Hook_Havok::IsCollisionEnabled(hkpCollidableCollidableFilter* a_this, bool* a_result, const hkpCollidable* a_collidableA, const hkpCollidable* a_collidableB) {
		a_result = _IsCollisionEnabled(a_this, a_result, a_collidableA, a_collidableB);
		if (*a_result) {
			auto colLayerA = GetCollisionLayer(a_collidableA);
			auto colLayerB = GetCollisionLayer(a_collidableB);

			//CollisionPrints(a_collidableA, a_collidableB);

			bool Check_A = (colLayerA == COL_LAYER::kBiped || colLayerA == COL_LAYER::kCharController || colLayerA == COL_LAYER::kDeadBip || colLayerA == COL_LAYER::kBipedNoCC);
			bool Check_B = (colLayerB == COL_LAYER::kBiped || colLayerB == COL_LAYER::kCharController || colLayerB == COL_LAYER::kDeadBip || colLayerB == COL_LAYER::kBipedNoCC);

			if (Check_A || Check_B) {
				auto objA = GetTESObjectREFR(a_collidableA);
				if (objA) {
					auto objB = GetTESObjectREFR(a_collidableB);
					if (objB) {
						if (objA != objB) {
							//Throw_ThrowCheck(objA, objB, colLayerA, colLayerB);
							if (IsCollisionDisabledBetween(objA, objB)) {
								*a_result = false;
							}
						}
					}
				}
			}
		}
		return a_result;
	}
}