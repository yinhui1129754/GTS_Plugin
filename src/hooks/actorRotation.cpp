#include "hooks/actorRotation.hpp"
#include "data/transient.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"
#include "utils/debug.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    float GetTinyRotation_X(Actor* actor) {
        float rotation_x = 0.0;
        auto transient = Transient::GetSingleton().GetData(actor);
        if (transient) {
            rotation_x = transient->Rotation_X;
        }
        return rotation_x;
    }

    float GetTinyRotation_X(TESObjectREFR* ref) {
        Actor* actor = skyrim_cast<Actor*>(ref);
        if (actor) {
            return GetTinyRotation_X(actor);
        }
        return 0.0;
    }
}

namespace Hooks { // This hook is commented out inside hooks.cpp

	void Hook_ActorRotation::Hook(Trampoline& trampoline) { // Sadly, works on player only and seems to depend on Camera Angle. Reports only Player.

        /*static FunctionHook<float(Actor* actor)> Skyrim_GetActorRotationX(  // 36601 = 1405EDD40 (SE), AE = ???
            REL::RelocationID(36601, 36601),
            [](auto* actor) {
                float result = Skyrim_GetActorRotationX(actor);
                float transient_rotation = GetTinyRotation_X(actor);
                if (transient_rotation != 0.0) {
                    result = transient_rotation;
                    log::info("Rotation != 0");
                }
                
                log::info("Rotation X of {} is: {}", actor->GetDisplayFullName(), result);
                
                
                return result;
            }
        );
    

    static FunctionHook<void(TESObjectREFR* ref, float x)> Skyrim_SetRefRotationX( // 19360 = 140296680 (SE)
        // Reports values properly on actor, but still does nothing.
        REL::RelocationID(19360, 19360),
        [](auto* ref, auto x) {
            //log::info("Raw Name Ref: {}", GetRawName(ref)); 
            //log::info("Pos: {}", Vector2Str(pos));
            float rotation = GetTinyRotation_X(ref);
            if (rotation != 0.0) {
                x = rotation;
                log::info("X of {} is {}", ref->GetDisplayFullName(), x);
            }
            
            return Skyrim_SetRefRotationX(ref, x);
        });*/
    }
}



