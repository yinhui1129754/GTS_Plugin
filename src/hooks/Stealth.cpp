#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "hooks/Stealth.hpp"
#include "scale/scale.hpp"
#include "utils/debug.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    float modify_detection(float in) {
        float modify = 1.0;
        if (in > 1e-6) {
            auto player = PlayerCharacter::GetSingleton();
            float scale = get_visual_scale(player);
            modify = 1.0 / scale;
        }
        return modify;
    }
    float modify_footstep_detection(Actor* giant, float in) {
        float scale = get_visual_scale(giant);
        float massscale = (scale * scale * scale);
        float modify = 0.0;
        if (HasSMT(giant)) {
            modify = (in * 4.0 + 80) * scale;
        } else {
            if (in > 1e-6) {
                modify = in * massscale;
            } else {
                modify = (1.0 * massscale) - 1.0;
            }
        }

        if (scale < get_natural_scale(giant)) {
            modify += in * massscale;
            modify -= 1.0 / massscale; // harder to hear small player
        }

        return modify;
    }
}


namespace Hooks {

	void Hook_Stealth::Hook(Trampoline& trampoline) { 
        
        // NEEDS AE OFFSET AND REL!
        static CallHook<float(Actor* giant)>CalculateFootstepDetection_1405FD870_5D0(
			REL::RelocationID(36758, 37774), REL::Relocate(0x2D4, 0x2D2), 
            // SE:
            //  0x1405FD870 : 36758
            //  0x1405fdb44 - 0x1405FD870 = 0x2d4 
            //  altering Character::GetEquippedWeight_1406195D0
            // AE:
            //  0x140625520 : 37774
            //  0x1406257f2 - 0x140625520 = 0x2D2
			[](auto* giant) {
				float result = CalculateFootstepDetection_1405FD870_5D0(giant); // Makes footsteps lounder for AI, works nicely so far
				if (giant->formID == 0x14 || IsTeammate(giant)) {
					//log::info("Hook Weight Result for {} is {}", giant->GetDisplayFullName(), result);
					float alter = modify_footstep_detection(giant, result);
					result = alter;
				}
				return result;
            }
        );
        
        static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading_var2(
			REL::RelocationID(36758, 37774), REL::Relocate(0x217, 0x217), 
            // SE:
            //  0x1405FD870 : 36758
            //  0x1405fda87 - 0x1405FD870 = 0x217 (line ~150)
            //  altering Character::GetHeading_1405FD780
            // AE:
            //  0x140625520 : 37774
            //  0x140625737 - 0x140625520 = 0x217 (wow same rel)
			[](auto* giant, auto* param_1) {
                float result = CalculateHeading_var2(giant, param_1);
                result *= modify_detection(result);
				return result;
            }
        );

        static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading_var3(
			REL::RelocationID(36758, 37774), REL::Relocate(0x92D, 0xA7F), 
            // SE:
            //  0x1405FD870 : 36758
            //  0x1405fe19d - 0x1405FD870 = 0x92D (line 370)
            //  altering Character::GetHeading_1405FD780
            // AE:
            //  0x140625520 : 37774
            //  0x140625f9f - 0x140625520 = 0xA7F
			[](auto* giant, auto* param_1) {
                float result = CalculateHeading_var3(giant, param_1);
                result *= modify_detection(result);
				return result;
            }
        );

        

        //////////////////////////////////////////////////TESTS

        /*static CallHook<float(uintptr_t param_1, uintptr_t param_2)>sub_1403BC410( // Seems to report how close we are to the someone
			REL::RelocationID(25812, 25812), REL::Relocate(0x24D, 0x24D), 
            // 25812
            // 0x1403bc65d - 0x1403BC410 = 0x24D
            // Altering thunk_powf_14134BEAC
			[](auto param_1, auto param_2) {
				float result = sub_1403BC410(param_1, param_2);
                //log::info("param 1: {}", GetRawName(param_1));
                //log::info("param 2: {}", GetRawName(param_1));
                //log::info("Test Hook value: {}", result);
				return result;
            }
        );

        static CallHook<float(ActorValueOwner* param_1, uintptr_t param_2)>GetAV_1( // Probably CTD
			REL::RelocationID(36758, 36758), REL::Relocate(0xE0, 0xE0), 
            // 36758
            // 0x1405fd950 - 0x1405FD870 = 0xE0
            // Function: 1405FD870, offset: 0x1405fd950
            // Altering thunk_powf_14134BEAC
			[](auto* param_1, auto param_2) {
				float result = GetAV_1(param_1, param_2);
                log::info("AV 1 hook called");
                log::info("AV 1 Result: {}", result);
                Actor* actor = skyrim_cast<Actor*>(param_1);
                if (actor) {
                    log::info("AV 2 actor: {}", actor->GetDisplayFullName());
                }
                // Reports 15. 15 = ActorValue::kSneak
                // But it also reports value of 100 if we change Sneak level skill through console to 5000 (cap probably)
				return result;
            }
        );

        static CallHook<float(ActorValueOwner* param_1, uintptr_t param_2)>GetAV_2( // Probably CTD
			REL::RelocationID(36758, 36758), REL::Relocate(0x427, 0x427), 
            // 36758
            // 0x1405fdc97 - 0x1405FD870 = 0x427
            // Function: 1405FD870, ofset: 0x1405fdc97
            // Altering thunk_powf_14134BEAC
			[](auto* param_1, auto param_2) {
				float result = GetAV_2(param_1, param_2);
                
                log::info("AV 2 hook called");
                log::info("AV 2Result: {}", result);
                Actor* actor = skyrim_cast<Actor*>(param_1);
                if (actor) {
                    log::info("AV 2 actor: {}", actor->GetDisplayFullName());
                }
                // Reports 15. 15 = ActorValue::kSneak
                // But it also reports value of 100 if we change Sneak level skill through console to 5000 (cap probably)
				return result;
            }
        );*/

        /*static CallHook<float(uintptr_t* param_1)>FUN_14085ddb0( // Probably CTD
			REL::RelocationID(50201, 50201), REL::Relocate(0x1BE, 0x1BE), 
            // 50201
            // 0x14085df6e - 0x14085ddb0 = 0x1BE
            // Function: FUN_14085ddb0, ofset: 0x14085df6e
            // Altering GetDetectionCalculatedValue_1405FC9A0
			[](auto* param_1) {
				float result = FUN_14085ddb0(param_1);
                
                log::info("GetDetectionCalculatedValue called");
                log::info("GetDetectionCalculatedValue result: {}", result);
                // reports a value but altering it does nothing
				return result;
            }
        );*/

        /*static FunctionHook<float(Actor* param_1)>GetDetectionCalculatedValue_1405FC9A0( REL::RelocationID(36748, 36748),
			[](auto* param_1) {
                float result = GetDetectionCalculatedValue_1405FC9A0(param_1);
                Actor* actor = skyrim_cast<Actor*>(param_1);
                if (actor) {
                    log::info("GetDetectionCalculatedValue_1405FC9A0 Actor: {}, result: {}", actor->GetDisplayFullName(), result);
                    Stealth_Stuff(actor);
                }
                // reports a value but altering it does nothing
				return result;
            }
        );*/
        

       /* static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading(
			REL::RelocationID(36758, 36758), REL::Relocate(0x71E, 0x71E), 
            //  0x1405fe19d - 0x1405FD870 = 0x71E (line 296)
            //  altering Character::GetHeading_1405FD780
			[](auto* giant, auto* param_1) {
				log::info("-- Heading Result for {}", giant->GetDisplayFullName());
                log::info("-------Heading param_1: {}", Vector2Str(param_1));
                log::info("-------Heading Result: {}", CalculateHeading(giant, param_1));
				return CalculateHeading(giant, param_1);
            }
        );

        static FunctionHook<float(Actor* giant, uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uintptr_t param_5,
			uintptr_t param_6, uintptr_t param_7, uintptr_t param_8, uintptr_t param_9, uintptr_t param_10)>
            CalculateDetection_1405FD870( REL::RelocationID(36758, 36758),
			[](auto* giant, auto param_2, auto param_3, auto param_4, auto param_5, auto param_6, auto param_7, auto param_8, auto param_9, auto param_10) {
                if (giant->formID == 0x14 || IsTeammate(giant)) {
                    log::info("- Hook Results for {}", giant->GetDisplayFullName());
                    log::info("------ Param_2 {}", param_2);
                    log::info("------ Param_3 {}", param_3);
                    log::info("------ Param_4 {}", param_4);
                    log::info("------ Param_5 {}", param_5);
                    log::info("------ Param_6 {}", param_6);
                    log::info("------ Param_7 {}", param_7);
                    log::info("------ Param_8 {}", param_8);
                    log::info("------ Param_9 {}", param_9);
                    log::info("------ Param_10 {}", param_10);
                }

                float result = CalculateDetection_1405FD870(giant, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
                result = 0.0;
				log::info("Hook Result: {}", result);
				return result;
            }
        );*/ // The general stealth hook.

        /*static FunctionHook<float(Actor* ref)>GetDetectionCalculatedValue( 
            REL::RelocationID(36748, 36748),
            [](auto* ref) {
				float result = 0.0;//GetDetectionCalculatedValue(ref);

				log::info("Detection of {} is {}", ref->GetDisplayFullName(), result);
				
                return result;
            }
        );*/// works but unknown what it does
    }
}