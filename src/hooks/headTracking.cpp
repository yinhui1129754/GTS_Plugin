#include "hooks/headTracking.hpp"
#include "utils/actorUtils.hpp"
#include "scale/modscale.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;


namespace {
	bool HasHeadTrackingTarget(Actor* giant) {
		auto process = giant->GetActorRuntimeData().currentProcess;
		if (process) {
			auto high = process->high;
			if (high) {
				if (process->GetHeadtrackTarget()) {
					return true;
				} else {
					return false;
				}
			}
		}
		return false;
	}

	bool KnockedDown(Actor* giant) {
		return static_cast<int>(giant->AsActorState()->GetKnockState()) != 0; // Another way of checking ragdoll just in case
	}

	bool IsinRagdollState(Actor* giant) {
		bool ragdolled = IsRagdolled(giant) || KnockedDown(giant);
		return ragdolled;
	}

	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			if (HasHeadTrackingTarget(giant)) { // enable it only when targeting someone
				if (IsinRagdollState(giant) || IsDragon(giant)) {  // Dragons behave funny if we edit them...sigh...
					// For some Bethesda™ reason - it breaks tiny ragdoll (their skeleton stretches :/) when they're small, so they fly into the sky.
					return original;      // We really want to prevent that, so we return original value.
				}
				float fix = original * ((get_giantess_scale(giant)) / game_getactorscale(giant)); // game_getscale() is used here by the game, so we want to / it again

				return fix;
			}
			// ^ Compensate it, since SetScale() already affects HT by default
		}
		return original;
	}

	void SetHeadtrackTargetImpl(Actor* actor, NiPoint3& target) {
		if (!actor) {
			return;
		}
		if (!HasHeadTrackingTarget(actor)) { // Only when target is nullptr
			if (IsinRagdollState(actor)) { // Needed to fix TDM bugs with deforming Meshes of Actors when we lock onto someone
				return;
			}
			// log::info("Actor: {}", actor->GetDisplayFullName());
			auto headPos = actor->GetLookingAtLocation();
			// log::info("headPos: {}", Vector2Str(headPos));
			auto model = actor->Get3D();
			if (!model) {
				return;
			}
			auto trans = model->world;
			auto transInv = trans.Invert();
			auto scale = get_visual_scale(actor);

			// log::info("headPos (local): {}", Vector2Str(transInv*headPos));
			auto unscaledHeadPos = trans * (transInv*headPos * (1.0/scale));
			// log::info("unscaledHeadPos: {}", Vector2Str(unscaledHeadPos));
			// log::info("unscaledHeadPos (local): {}", Vector2Str(transInv*headPos));
			auto direction = target - headPos;
			// log::info("direction: {}", Vector2Str(direction));
			target = unscaledHeadPos + direction;
		}
	}
}

namespace Hooks
{

	void Hook_HeadTracking::Hook(Trampoline& trampoline) {
		static CallHook<float(TESObjectREFR* param_1)>Alter_Headtracking( 
			REL::RelocationID(37129, 37364), REL::Relocate(0x24, 0x5E),
			[](auto* param_1) {
				// Applied only when we TDM track someone
				// ----------------- SE:
				// FUN_140615030 : 37129
				// 0x140615054 - 0x140615030 = 0x24

				//------------------ AE:
				// FUN_1405ffc50: 37364
				// 0x1405ffcae - 0x1405ffc50 = 0x5E
  
				float result = Alter_Headtracking(param_1);
				float alter = affect_by_scale(param_1, result); 

				return alter;
            }
        );

		static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> 
			SetHeadtrackTarget(RELOCATION_ID(38850, 39887),
				[](auto* a_this, auto* a_owner, auto& a_targetPosition) {
				// Applied in all other cases
				SetHeadtrackTargetImpl(a_owner, a_targetPosition);
				SetHeadtrackTarget(a_this, a_owner, a_targetPosition);
				return;
			}
		);
	}
}

