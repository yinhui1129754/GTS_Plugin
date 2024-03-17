#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "data/transient.hpp"
#include "hooks/Movement.hpp"
#include "scale/scale.hpp"


using namespace RE;
using namespace SKSE;

namespace {
	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			float scale = get_giantess_scale(giant);
			return scale;
		}
		return original;
	}
}

namespace Hooks
{
    void Hook_Movement::Hook(Trampoline& trampoline) {

        static CallHook<float(TESObjectREFR* param_1)>Scale_AlterMovementSpeed(  // Movement speed alteration
            REL::RelocationID(37013, 37943), REL::Relocate(0x1A, 0x51),          
            [](auto* param_1) {
                // ---------------SE:
                // sub_14060EEF0 : 37013            
                // 0x14060ef0a - 0x14060EEF0 = 0x1A      
                // ---------------AE:
                // FUN_140630510 :  37943
                // 0x140630561 - 0x140630510 = 0x51
                float result = 1.0; // force it to 1.0. We DON'T want the SetScale() to affect it.
                //log::info("(21) - Hooked Alter Movement Speed, value * 0.15");
                return result;
            }
        );


        /*static CallHook<float(TESObjectREFR* param_1)>sub_140623F10( // Seems to be called on attacks. 
            REL::RelocationID(37588, 37588), REL::Relocate(0x6B, 0x6B), // Supposedly moves invisible "Hitbox" zone for weapons more forward or something
            [](auto* param_1) {                                         // Not sure.
                // 37588
                // 0x140623f7b - 0x140623F10 = 0x6B
                float result = sub_140623F10(param_1);
                float Alter = affect_by_scale(param_1, result);
                log::info("(18) sub_140623F10 Hooked");
                return Alter;
            }
        );
        //^ Hook 18

        static CallHook<float(TESObjectREFR* param_1)>sub_1404E6B30_1(
            REL::RelocationID(31951, 31951), REL::Relocate(0x1F9, 0x1F9),
            [](auto* param_1) {
                // 31951
                // 0x1404e6d29 - 0x1404E6B30 = 0x1F9
                float result = sub_1404E6B30_1(param_1);
                float Alter = affect_by_scale(param_1, result);
                //log::info("(42 - 1) sub_1404E6B30 Hooked");
                return Alter;
            }
        );
        //^ Hook 42

        static CallHook<float(TESObjectREFR* param_1)>sub_1404E6B30_2(
            REL::RelocationID(31951, 31951), REL::Relocate(0xDD, 0xDD),
            [](auto* param_1) {
                // 31951
                // 0x1404e6c0d - 0x1404E6B30 = 0xDD
                float result = sub_1404E6B30_2(param_1);
                float Alter = affect_by_scale(param_1, result);
                //log::info("(42 - 2) sub_1404E6B30 Hooked");
                return Alter;
            }
        );
        //^ Hook 43


        static CallHook<float(TESObjectREFR* param_1)>sub_1404E6B30_3(
            REL::RelocationID(31951, 31951), REL::Relocate(0x5B, 0x5B),
            [](auto* param_1) {
                // 31951
                // 0x1404e6b8b - 0x1404E6B30 = 0x5B
                float result = sub_1404E6B30_3(param_1);
                float Alter = affect_by_scale(param_1, result);
                //log::info("(42 - 3) sub_1404E6B30 Hooked");
                return Alter;
            }
        );
        //^ Hook 44

        static CallHook<float(TESObjectREFR* param_1)>sub_1407BAB40(     // Seems to work after hitting someone/when we're detected. Disabling hostile actor stops prints.
			REL::RelocationID(46018, 46018), REL::Relocate(0x15, 0x15), 
			// 46018
			// 0x1407bab55 - 0x1407BAB40 = 0x15
			[](auto* param_1) {
				float result = sub_1407BAB40(param_1);
                float Alter = affect_by_scale(param_1, result);
				//log::info("(5) sub_1407BAB40 Hooked");	
				return Alter;
            }
        ); */
    }
}