#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchObject.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
    void ApplyPhysicsToObject(Actor* giant, TESObjectREFR* object, NiPoint3 push, float force, float scale) {
		force *= GetLaunchPower_Object(scale, true); // Should be * 0.40

		if (Runtime::HasPerkTeam(giant, "DisastrousTremor")) {
			force *= 1.5;
		}

		NiAVObject* Node = object->Get3D1(false);
		if (Node) {
			auto collision = Node->GetCollisionObject();
			if (collision) {
				auto rigidbody = collision->GetRigidBody();
				if (rigidbody) {
					auto body = rigidbody->AsBhkRigidBody();
					auto motion = skyrim_cast<hkpMotion*>(rigidbody);
					
					if (body) {
						// Goal: Somehow read  hkpMotion->GetMass()
						//log::info("Applying force to object, Push: {}, Force: {}, Result: {}", Vector2Str(push), force, Vector2Str(push * force));
						SetLinearImpulse(body, hkVector4(push.x * force, push.y * force, push.z * force, 1.0));
					}
				}
			}
		}
	}
}


namespace Gts {
    float GetLaunchPower_Object(float sizeRatio, bool Launch) {
		// https://www.desmos.com/calculator/wh0vwgljfl
		if (Launch) {
			SoftPotential launch {
				.k = 1.6,
				.n = 0.62,
				.s = 0.6,
				.a = 0.0,
			};
			return soft_power(sizeRatio, launch);
		} else {
			SoftPotential kick {
			.k = 1.42,
			.n = 0.78,
			.s = 0.6,
			.a = 0.0,
			};
			return soft_power(sizeRatio, kick);
		}
	}

    void PushObjectsUpwards(Actor* giant, std::vector<NiPoint3> footPoints, float maxFootDistance, float power) {
		auto profiler = Profilers::Profile("Other: Launch Objects");
		bool AllowLaunch = Persistent::GetSingleton().launch_objects;
		if (!AllowLaunch) {
			return;
		}

		float giantScale = get_visual_scale(giant);

		float start_power = Push_Object_Upwards * (1.0 + Potion_GetMightBonus(giant));

		if (Runtime::HasPerkTeam(giant, "DisastrousTremor")) {
			power *= 1.5;
		}
		std::vector<ObjectRefHandle> Refs = GetNearbyObjects(giant);

		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
			DebugAPI::DrawSphere(glm::vec3(footPoints[0].x, footPoints[0].y, footPoints[0].z), maxFootDistance, 600, {0.0, 1.0, 0.0, 1.0});
		}

        for (auto object: Refs) {
			if (object) {
				auto objectref = object.get().get();
				if (objectref) {
					if (objectref && objectref->Is3DLoaded()) {
						NiPoint3 objectlocation = objectref->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - objectlocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;
								float push = start_power * GetLaunchPower_Object(giantScale, false) * force * power;
								auto Object1 = objectref->Get3D1(false);
								if (Object1) {
									auto collision = Object1->GetCollisionObject();
									if (collision) {
										auto rigidbody = collision->GetRigidBody();
										if (rigidbody) {
											auto body = rigidbody->AsBhkRigidBody();
											if (body) {
												SetLinearImpulse(body, hkVector4(0, 0, push, push));
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
    }
            
    void PushObjectsTowards(Actor* giant, TESObjectREFR* object, NiAVObject* Bone, float power, float radius, bool Kick) {
		auto profiler = Profilers::Profile("Other: Launch Objects");
		bool AllowLaunch = Persistent::GetSingleton().launch_objects;
		if (!AllowLaunch) {
			return;
		}

		if (!Bone) {
			return;
		} 
		if (!object) {
			return;
		}
        if (!object->Is3DLoaded()) {
			return;
        }
        if (!object->GetCurrent3D()) {
            return;
        }

		float giantScale = get_visual_scale(giant);

		float start_power = 0.10 * Push_Object_Forward * (1.0 + Potion_GetMightBonus(giant));

		NiPoint3 point = Bone->world.translate;
		float maxDistance = radius * giantScale;

        if (Kick) { // Offset pos down
            float HH = HighHeelManager::GetHHOffset(giant).Length();
			point.z -= HH * 0.75;
		}

		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
			DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance, 200, {0.5, 0.0, 0.5, 1.0});
		}

		int nodeCollisions = 0;

		float distance = (point - object->GetPosition()).Length();
		if (distance < maxDistance) {
		
			float Start = Time::WorldTimeElapsed();
			ActorHandle gianthandle = giant->CreateRefHandle();
			ObjectRefHandle objectref = object->CreateRefHandle();
			std::string name = std::format("PushObject_{}_{}", giant->formID, object->formID);

			NiPoint3 StartPos = Bone->world.translate;

			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				} if (!objectref) {
					return false;
				}
				auto giantref = gianthandle.get().get();
				auto ref = objectref.get().get();
				float Finish = Time::WorldTimeElapsed();
				float timepassed = Finish - Start;

				if (timepassed > 1e-4) {
					NiPoint3 EndPos = Bone->world.translate;
					ApplyPhysicsToObject(giantref, ref, EndPos - StartPos, start_power, giantScale);
					return false; // end it
				}
				return true;
			});
		}
	}

	void PushObjects(std::vector<ObjectRefHandle> refs, Actor* giant, NiAVObject* bone, float power, float radius, bool Kick) {
		if (!refs.empty()) {
			for (auto object: refs) {
				if (object) {
					TESObjectREFR* objectref = object.get().get();
					PushObjectsTowards(giant, objectref, bone, power, radius, Kick);
				}
			}
		}
	}

	std::vector<ObjectRefHandle> GetNearbyObjects(Actor* giant) {
		bool AllowLaunch = Persistent::GetSingleton().launch_objects;
		if (!AllowLaunch) {
			return {};
		}
		float giantScale = get_visual_scale(giant);

		float maxDistance = 220 * giantScale;

		std::vector<ObjectRefHandle> Objects = {};
		NiPoint3 point = giant->GetPosition();

		bool PreciseScan = Runtime::GetBoolOr("AccurateCellScan", false);

		if (!PreciseScan) { // Scan single cell only
			TESObjectCELL* cell = giant->GetParentCell();
			if (cell) {
				auto data = cell->GetRuntimeData();
				for (auto object: data.references) {
					if (object) {
						auto objectref = object.get();
						if (objectref) {
							bool IsActor = objectref->Is(FormType::ActorCharacter);
							if (!IsActor) { // we don't want to apply it to actors
								NiPoint3 objectlocation = objectref->GetPosition();
								float distance = (point - objectlocation).Length();
								if (distance <= maxDistance) {
									ObjectRefHandle refhandle = objectref->CreateRefHandle(); 
									Objects.push_back(refhandle);
								}
							}
						}
					}
				}
			}
		} else if (PreciseScan && REL::Module::IsSE()) { // Else scan Entire world, SE only for now: TES->ForEachReferenceInRange crashes on AE!
			const auto TES = TES::GetSingleton(); // Crashes on AE, ty Todd (Also seems to be FPS expensive)
			if (TES) {
				TESObjectREFR* GiantRef = skyrim_cast<TESObjectREFR*>(giant);
				if (GiantRef) {
					TES->ForEachReferenceInRange(GiantRef, maxDistance, [&](RE::TESObjectREFR& a_ref) {
						bool IsActor = a_ref.Is(FormType::ActorCharacter);
						if (!IsActor) { // we don't want to apply it to actors
							NiPoint3 objectlocation = a_ref.GetPosition();
							float distance = (point - objectlocation).Length();
							if (distance <= maxDistance) {
								ObjectRefHandle handle = a_ref.CreateRefHandle();
								Objects.push_back(handle);
							}
						}
						return RE::BSContainer::ForEachResult::kContinue;    
					});
				}
			}
		}

		return Objects;
	}
}