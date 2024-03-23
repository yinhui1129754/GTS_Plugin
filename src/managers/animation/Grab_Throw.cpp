#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/Grab_Throw.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/OverkillManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;
namespace {

    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void Throw_Actor(ActorHandle giantHandle, ActorHandle tinyHandle, NiPoint3 startCoords, NiPoint3 endCoords, std::string_view TaskName) {

		double startTime = Time::WorldTimeElapsed();

		TaskManager::Run(TaskName, [=](auto& update){
			if (!giantHandle) {
				return false;
			}
			if (!tinyHandle) {
				return false;
			}
			Actor* giant = giantHandle.get().get();
			Actor* tiny = tinyHandle.get().get();
			
			// Wait for 3D to be ready
			if (!giant->Is3DLoaded()) {
				return true;
			}
			if (!giant->GetCurrent3D()) {
				return true;
			}
			if (!tiny->Is3DLoaded()) {
				return true;
			}
			if (!tiny->GetCurrent3D()) {
				return true;
			}

			double endTime = Time::WorldTimeElapsed();

			if ((endTime - startTime) > 0.05) {
				// Enough Time has elapsed

				float Scale = get_visual_scale(giant) * GetSizeFromBoundingBox(giant);

				// Calculate power of throw

				NiPoint3 direction = NiPoint3();
				NiPoint3 vector = endCoords - startCoords;

				float distanceTravelled = vector.Length();
				float timeTaken = endTime - startTime;
				float speed = (distanceTravelled/timeTaken) * 36; // Standing throw default power

				if (!giant->IsSneaking()) { // Goal is to fix standing throw direction

					float angle_x = 10; // Runtime::GetFloat("cameraAlternateX"); // 10
					float angle_y = 0; // Runtime::GetFloat("cameraAlternateY");//0.0;
					float angle_z = 0;//::GetFloat("combatCameraAlternateX"); // 0

					// Conversion to radians
					const float PI = 3.141592653589793;
					float angle_x_rad = angle_x * 180.0 / PI;
					float angle_y_rad = angle_y * 180.0 / PI;
					float angle_z_rad = angle_z * 180.0 / PI;

					// Work out direction from angles and an initial (forward) vector;
					//
					// If all angles are zero then it goes forward
					// angle_x is pitch
					// angle_y is yaw
					// angle_z is roll
					//
					// The order of operation is pitch > yaw > roll
					NiMatrix3 customRot = NiMatrix3(angle_x_rad, angle_y_rad, angle_z_rad);
					NiPoint3 forward = NiPoint3(0.0, 0.0, 1.0);
					NiPoint3 customDirection = customRot * forward;

					NiMatrix3 giantRot = giant->GetCurrent3D()->world.rotate;
					direction = giantRot * (customDirection / customDirection.Length());
				} else {
				    if (IsCrawling(giant)) { // Strongest throw, needs custom throw direction again
						speed *= 0.66; // Hand travels fast so it's a good idea to decrease its power

						float angle_x = 0;//Runtime::GetFloat("cameraAlternateX"); // 0
						float angle_y = 0.008; // Runtime::GetFloat("cameraAlternateY");//0.008;
						float angle_z = 0.0;// Runtime::GetFloat("combatCameraAlternateX"); // 0

						// Conversion to radians
						const float PI = 3.141592653589793;
						float angle_x_rad = angle_x * 180.0 / PI;
						float angle_y_rad = angle_y * 180.0 / PI;
						float angle_z_rad = angle_z * 180.0 / PI;

						NiMatrix3 customRot = NiMatrix3(angle_x_rad, angle_y_rad, angle_z_rad);
						NiPoint3 forward = NiPoint3(0.0, 0.0, 1.0);
						NiPoint3 customDirection = customRot * forward;

						NiMatrix3 giantRot = giant->GetCurrent3D()->world.rotate;
						direction = giantRot * (customDirection / customDirection.Length());
					} else { // Else perform Slight Sneak Throw calc
						direction = vector / vector.Length();
						speed *= 0.33; // Hand also travels fast and we don't want this anim to feel strong
					}
				}

				float Time = (1.0 / Time::GetTimeMultiplier()); // read SGTM value and / speed by it, so tinies still fly far even with sgtm 0.15
				log::info("Time Mult: {}", Time);


				ApplyManualHavokImpulse(tiny, direction.x, direction.y, direction.z, speed * Time);
				return false;
			} 
			return true;
		});
	}
	

	void Throw_DoCollisionDamage(TESObjectREFR* victim_ref, TESObjectREFR* aggressor_ref, float speed) {
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

	void Throw_RayCastTask(Actor* giant, Actor* tiny, float speed) {
		// currently does nothing
		// Throw_DoCollisionDamage(victim_ref, aggressor_ref, speed);
		// Idea is to apply damage when actor collides with the ground/rock and inflict damage based on how long it took to hit something
	}

	void Throw_RegisterForThrowDamage(Actor* giant, Actor* tiny, float speed) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			//Throw_RayCastTask(giant, tiny, speed);
			transient->Throw_WasThrown = true;
			transient->Throw_Offender = giant;
			transient->Throw_Speed = speed;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// E V E N T S
	/////////////////////////////////////////////////////////////////////////////////

    void GTSGrab_Throw_MoveStart(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabThrow", "DestructionBasics", true, 1.25);
		ManageCamera(giant, true, CameraTracking::Grab_Left);
		StartLHandRumble("GrabThrowL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Throw_FS_R(AnimationEventData& data) {
		if (IsUsingThighAnimations(&data.giant) || IsCrawling(&data.giant)) {
			return; // Needed to not apply it during animation blending for thigh/crawling animations
		}
		float shake = 1.0;
		float launch = 1.0;
		float dust = 0.9;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		GRumble::Once("StompR", &data.giant, 1.50 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.1 * launch * data.animSpeed * perk, 1.0 * launch * data.animSpeed, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * perk, 1.25, FootEvent::Right);
	}

	void GTSGrab_Throw_FS_L(AnimationEventData& data) {
		if (IsUsingThighAnimations(&data.giant) || IsCrawling(&data.giant)) {
			return; // Needed to not apply it during animation blending for thigh/crawling animations
		}
		float shake = 1.0;
		float launch = 1.0;
		float dust = 0.9;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		GRumble::Once("StompL", &data.giant, 1.50 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.1 * launch * data.animSpeed * perk, 1.0 * launch * data.animSpeed, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * perk, 1.25, FootEvent::Left);
	}

	void GTSGrab_Throw_Throw_Pre(AnimationEventData& data) {// Throw frame 0
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);

		Grab::DetachActorTask(giant);
		Grab::Release(giant);

		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);

		if (otherActor) {

			auto charcont = otherActor->GetCharController();
			if (charcont) {
				charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
			}

			auto bone = find_node(giant, "NPC L Hand [LHnd]"); 
			if (bone) {
				NiPoint3 startCoords = bone->world.translate;

				ActorHandle gianthandle = giant->CreateRefHandle();
				ActorHandle tinyhandle = otherActor->CreateRefHandle();

				std::string name = std::format("Throw_{}_{}", giant->formID, otherActor->formID);
				std::string pass_name = std::format("ThrowOther_{}_{}", giant->formID, otherActor->formID);
				// Run task that will actually launch the Tiny
				TaskManager::Run(name, [=](auto& update){
				if (!gianthandle) {
					return false;
				}
				if (!tinyhandle) {
					return false;
				}
				Actor* giant = gianthandle.get().get();
				Actor* tiny = tinyhandle.get().get();
				
				// Wait for 3D to be ready
				if (!giant->Is3DLoaded()) {
					return true;
				}
				if (!giant->GetCurrent3D()) {
					return true;
				}
				if (!tiny->Is3DLoaded()) {
					return true;
				}
				if (!tiny->GetCurrent3D()) {
					return true;
				}

				NiPoint3 endCoords = bone->world.translate;

				SetBeingHeld(tiny, false);
				EnableCollisions(tiny);

				PushActorAway(giant, tiny, 1.0);

				auto charcont = tiny->GetCharController();
				if (charcont) {
					charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Stop actor moving in space, just in case
				}
				Throw_Actor(gianthandle, tinyhandle, startCoords, endCoords, pass_name);
				
				return false;
				});
			}
		}
	}
	

	void GTSGrab_Throw_ThrowActor(AnimationEventData& data) { // Throw frame 1
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);

		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		ManageCamera(giant, false, CameraTracking::Grab_Left);
		GRumble::Once("ThrowFoe", &data.giant, 2.50, 0.10, "NPC L Hand [LHnd]");
		AnimationManager::StartAnim("TinyDied", giant);

		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	void GTSGrab_Throw_Throw_Post(AnimationEventData& data) { // Throw frame 2
	}

	void GTSGrab_Throw_MoveStop(AnimationEventData& data) { // Throw Frame 3
		auto giant = &data.giant;
		DrainStamina(giant, "GrabThrow", "DestructionBasics", false, 1.25);
		StopLHandRumble("GrabThrowL", data.giant);
	}

}

namespace Gts {
    void Animation_GrabThrow::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSGrab_Throw_MoveStart", "Grabbing", GTSGrab_Throw_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Throw_FS_R", "Grabbing", GTSGrab_Throw_FS_R);
		AnimationManager::RegisterEvent("GTSGrab_Throw_FS_L", "Grabbing", GTSGrab_Throw_FS_L);
		AnimationManager::RegisterEvent("GTSGrab_Throw_Throw_Pre", "Grabbing", GTSGrab_Throw_Throw_Pre);
		AnimationManager::RegisterEvent("GTSGrab_Throw_ThrowActor", "Grabbing", GTSGrab_Throw_ThrowActor);
		AnimationManager::RegisterEvent("GTSGrab_Throw_Throw_Post", "Grabbing", GTSGrab_Throw_Throw_Post);
		AnimationManager::RegisterEvent("GTSGrab_Throw_MoveStop", "Grabbing", GTSGrab_Throw_MoveStop);
    }
}