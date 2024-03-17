#include "hooks/Pushback.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"

namespace {

    float GetPushMult(Actor* giant) {
        auto tranData = Transient::GetSingleton().GetData(giant);

        float result = 1.0;

        if (tranData) {
            result = tranData->push_force;
        } else {
            float size = get_giantess_scale(giant);
            if (HasSMT(giant)) {
			    size *= 2.0;
		    }
            result = std::clamp(1.0f / (size*size*size), 0.01f, 1.0f);
        }

		if (result <= 0.01) {
			return 0.0;
		}

		return result;
	}
}
namespace Hooks
{
	void Hook_Pushback::Hook(Trampoline& trampoline) {

        static FunctionHook<void(bhkCharacterController* controller, hkVector4& a_from, float time)>HavokPushHook (      
			REL::RelocationID(76442, 78282), 
			[](bhkCharacterController* controller, hkVector4& a_from, float time) { // SE: DC0930
				// TO-DO: Somwehow improve performance instead of looping through all actors
                auto profiler = Profilers::Profile("Hook: HavokPush");
				Actor* giant = GetCharContActor(controller);
				float scale = 1.0;
				if (giant) {
                    if (giant->formID == 0x14) {
                        log::info("Giant found: {}", giant->GetDisplayFullName());
                        log::info("HavokPush");
                        log::info("a_from: {}", Vector2Str(a_from));
                        log::info("time: {}", time);
                    }
					
					scale = GetPushMult(giant);
				}
				hkVector4 Push = hkVector4(a_from) * scale;
				
				return HavokPushHook(controller, Push, time); 
            }
        );

		/*static FunctionHook<void(AIProcess *ai, Actor* actor, NiPoint3& direction, float force)>PushActorAwayHook (      
			REL::RelocationID(38858, 39895), 
			[](AIProcess *ai, Actor* actor, NiPoint3& direction, float force) { // Use it to cache size difference between 2 actors
				
				//Cache size difference and then use it inside hook above
				return PushActorAwayHook(ai, actor, direction, force); 
            }
        );*/
    }
}