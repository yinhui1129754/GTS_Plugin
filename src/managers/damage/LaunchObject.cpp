#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchObject.hpp"
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

#include "managers/TES.hpp"

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
		const float start_power = 0.4;

		force *= start_power * GetLaunchPower_Object(scale, true);

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
					if (body) {
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

		float start_power = 0.5;

		if (Runtime::HasPerkTeam(giant, "DisastrousTremor")) {
			power *= 1.5;
		}
		std::vector<TESObjectREFR*> Refs = GetNearbyObjects(giant);

        for (auto objectref: Refs) {
            if (objectref) {
                if (objectref && objectref->Is3DLoaded()) {
                    if (objectref->GetCurrent3D()) {
                        bool IsActor = objectref->Is(FormType::ActorCharacter);
                        if (!IsActor) { // we don't want to apply it to actors
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
		float force = 0.25;

		float distance = (point - object->GetPosition()).Length();
		if (distance < maxDistance) {
		
			float Start = Time::WorldTimeElapsed();
			ActorHandle gianthandle = giant->CreateRefHandle();
			std::string name = std::format("PushObject_{}_{}", giant->formID, object->formID);

			NiPoint3 StartPos = Bone->world.translate;

			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				}
				auto giantref = gianthandle.get().get();
				float Finish = Time::WorldTimeElapsed();
				float timepassed = Finish - Start;

				if (timepassed > 1e-4) {
					NiPoint3 EndPos = Bone->world.translate;
					ApplyPhysicsToObject(giantref, object, EndPos - StartPos, force, giantScale);
					return false; // end it
				}
				return true;
			});
		}
	}

	void PushObjects(std::vector<TESObjectREFR*> refs, Actor* giant, NiAVObject* bone, float power, float radius, bool Kick) {
		if (!refs.empty()) {
			for (auto object: refs) {
				if (object) {
					PushObjectsTowards(giant, object, bone, power, radius, Kick);
				}
			}
		}
	}

	std::vector<TESObjectREFR*> GetNearbyObjects(Actor* giant) {
		bool AllowLaunch = Persistent::GetSingleton().launch_objects;
		if (!AllowLaunch) {
			return {};
		}
		float giantScale = get_visual_scale(giant);

		float maxDistance = 220 * giantScale;

		std::vector<TESObjectREFR*> Objects = {};
		NiPoint3 point = giant->GetPosition();

        const auto TES = Gts::TES::GetSingleton();
		if (TES) {
			TESObjectREFR* GiantRef = skyrim_cast<TESObjectREFR*>(giant);
			if (GiantRef) {
				TES->ForEachReferenceInRange(GiantRef, maxDistance, [&](RE::TESObjectREFR& a_ref) {
					bool IsActor = a_ref.Is(FormType::ActorCharacter);
					if (!IsActor) { // we don't want to apply it to actors
						NiPoint3 objectlocation = a_ref.GetPosition();
						float distance = (point - objectlocation).Length();
						if (distance <= maxDistance) {
							Objects.push_back(&a_ref);
						}
					}
					return RE::BSContainer::ForEachResult::kContinue;    
				});
			}
		}

		return Objects;
	}
}