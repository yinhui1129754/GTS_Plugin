#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "data/transient.hpp"
#include "hooks/Experiments.hpp"
#include "scale/scale.hpp"


using namespace RE;
using namespace SKSE;


// Possible hooks that may benefit from scaling: All info for Special Edition (SE)
// Main REF: TESObjectREFR::GetScale_14028CC60
//  [x] = attempted to hook it
// -[x] Character::GetEyeHeight_140601E40  									(Barely prints anything with unknown conditions. Useless)  
// -[x] Actor::Jump_1405D1F80              									Jump Height. We already hook that.
// -[x] Pathing::sub_140474420             									(perhaps pathing fix)  [Looks like it does nothing, there's no prints]
// -[x] TESObjectREFR::sub_140619040       									 Offset:  0x17E        37323     
// -[x] IAnimationGraphManagerHolder::Func7_140609D50      					 Offset:  0xBD
// - FUN_1405513a0                                      					(Something attack angle related)
// -[x] NiNode::sub_1402AA350(NiNode *param_1)                           	 Offset:  0xBC
// - TESObject::LoadGraphics_140220DD0(TESObject *param_1)                   Offset:  0x1FC
// -[x] TESObjectREFR::sub_1407BA9C0                                         offset:  0x57         46015     
// -[x] TESObjectREFR::sub_1407BA910                                         offset:  0x94         46014 
// -[x] TESObjectREFR::sub_1407BA8B0                                         offset:  0x44         46013  
// -[x] TESObjectREFR::sub_1406AA410                                         offset:  0xC1         39477
// ^ These 4 subs seem to do literally nothing.

//      TESObjectREFR::sub_1405FE650
//      Pathing::sub_140473120(Pathing *param_1,uint64 param_2,Character *param_3,uint64 param_4) (Seems to be called in lots of places
//      ^ 29819
//      Pathing::sub_140473490(Pathing *param_1,uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uint64 param_5)
//      ^ 29824
//      Pathing::sub_140473200(Pathing *param_1,uint64 param_2,Character *param_3,uint64 param_4)
//      ^ 29820
//      PlayerCamera::Update_14084AB90
//      FUN_14085c290(uint64 param_1,char *param_2,uint64 param_3,uint64 param_4)
//      ^ 50179

namespace {
	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			float scale = get_giantess_scale(giant);
			return scale;
		}
		return original;
	}
	float camera_getplayersize() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float size = get_visual_scale(player);
			if (size > 1e-6) {
				return size;
			}
		}
		return 1.0;
	}

	bool AllowAttack(TESActionData* data) { // Credits to Kaputt source code (by Pentalimbed)
		auto attacker = data->source->As<RE::Actor>();
    	if (!attacker) {
			log::info("Attacker false");
         	return true;
		}
		log::info("Found Attacker: {}", attacker->GetDisplayFullName());

		auto receiverRef = attacker->GetActorRuntimeData().currentCombatTarget;
		if (!receiverRef) {
			log::info("Receiver false");
			return true;
		}
		auto receiver = receiverRef.get().get();
		log::info("Found Receiver: {}", receiver->GetDisplayFullName());

		float size_difference = get_giantess_scale(receiver)/get_giantess_scale(attacker);
		if (size_difference >= 1.15) {
			log::info("Size Difference > 1.15. Disallow");
			attacker->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kIsInKillMove);
			receiver->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kIsInKillMove);
			return false;
		}

		return true;
	}
}


namespace Hooks {

	void Hook_Experiments::Hook(Trampoline& trampoline) { // This hook is commented out inside hooks.cpp
		//							  																
		//																							
		//  HitFrameHandler::Handle_1407211B0 & BSTaskPool_HandleTask_1405C6EE0 -> Actor::sub_140627930 
		//  CalculateCurrentHitTargetForWeaponSwing_140629090
		//  Character::HitData_140628C20 ( called by Actor::sub_140627930 )
		//  DoCombatSpellApply_1406282E0
		//  FUN_14062b870                         															
		//	Actor::sub_140627930
		/*static CallHook<bool(TESActionData* param_1)>ActionDataHook (  // Credits to Kaputt source code (by Pentalimbed)
			REL::RelocationID(48139, 49170), REL::Relocate(0x4d7, 0x435),         
			// Allows/disallows NPC's to perform attack Animation
			// Sadly doesn't affect KillMove
			[](auto* param_1) {
				// sub_14060EEF0 : 37013
				// 0x14060ef0a - 0x14060EEF0 = 0x1A
				bool result = AllowAttack(param_1);
				if (result) {
					result = ActionDataHook(param_1);
				}
				log::info("Allow: {}", result);
				return result;
            }
        );*/

		/*static FunctionHook<float(bhkCharacterController* param_1, uintptr_t param_2)>CheckLayerPenetration (        
			REL::RelocationID(76469, 76469), 
			// 140dc35c0
			[](auto* param_1, auto param_2) {
				log::info("CheckLayerPenetration Hook");
				log::info("Param 2:{}", param_2);
				float result = CheckLayerPenetration(param_1, param_2);
				log::info("result: {}", result);
				return CheckLayerPenetration(param_1, param_2); 
            }
        );
		// Members of MoveHavok

		static FunctionHook<void(bhkCharacterController* param_1, hkVector4* param_2, uintptr_t param_3)>sub_140DC0C90 (        
			REL::RelocationID(76452, 76452), 
			[](auto* param_1, hkVector4* param_2, auto param_3) {
				log::info("sub_140DC0C90");
				log::info("--Param 2: {}", Vector2Str(param_2)); 
				log::info("--Param 3: {}", param_3);

				param_2->quad.m128_f32[0] *= 0.0;
				param_2->quad.m128_f32[1] *= 0.0;
				param_2->quad.m128_f32[2] *= 0.0;
				param_2->quad.m128_f32[3] *= 0.0;
			
				return sub_140DC0C90(param_1, param_2, param_3);  
            }
        );

		static FunctionHook<void(bhkCharacterController* param_1, hkVector4* param_2)>sub_1402A22F0 (        
			REL::RelocationID(19674, 19674), 
			[](auto* param_1, auto param_2) {
				log::info("sub_1402A22F0");
				log::info("--Param 2: {}", Vector2Str(param_2));
				return sub_1402A22F0(param_1, param_2); 
            }
        );

		static FunctionHook<void(bhkCharacterController* param_1, hkVector4* param_2)>sub_140DBEE70 (        
			REL::RelocationID(76422, 76422), 
			[](auto* param_1, auto param_2) {
				log::info("sub_140DBEE70");
				log::info("--Param 2: {}", Vector2Str(param_2));
				return sub_140DBEE70(param_1, param_2); 
            }
        );

		/*static FunctionHook<float(uintptr_t param_1, uintptr_t* param_2)>FUN_140c32030 (  // Called by Havok hook (38112, 39068)}; // SE: 6403D0
			REL::RelocationID(67997, 67997), 
			[](auto param_1, auto* param_2) {
				log::info("FUN_140c32030");
				float result = FUN_140c32030(param_1, param_2);
				log::info("--Result: {}", result);
				return FUN_140c32030(param_1, param_2); 
            }
        );*/

		//sub_140DC0C90(bhkCharacterController* param_1, NiPoint3* param_2, uintptr_t param_3); // 76452
		//sub_1402A22F0(bhkCharacterController* param_1, NiPoint3* param_2); // 19674
		//sub_140DBEE70(bhkCharacterController* param_1, uintptr_t param_2); // 76422

		/*static FunctionHook<bool(AIProcess* AI, Actor* a_actor, DEFAULT_OBJECT a_action, TESIdleForm* a_idle, uintptr_t a_arg5, uintptr_t a_arg6, TESObjectREFR* a_target)>AnimationHook (        
			REL::RelocationID(38290, 39256), 
			[](auto* AI, auto* a_actor, auto a_action, auto* a_idle, uintptr_t a_arg5, uintptr_t a_arg6, auto* a_target) {
				
				log::info("Animation Hook");
				if (AI) {
					log::info("AI true");
				}
				if(a_actor) {
					log::info("Actor: {}", a_actor->GetDisplayFullName());
				}
				if (a_action) {
					log::info("a_action: {}", a_action);
				}
				if (a_idle) {
					log::info("idle: {}", a_idle->GetFormEditorID());
				}
				//log::info("arg5: {}", a_arg5);
				//log::info("arg6: {}", a_arg6);
				if (a_target) {
					log::info("target: {}", a_target->GetDisplayFullName());
				}
				return AnimationHook(AI, a_actor, a_action, a_idle, a_arg5, a_arg6, a_target);   // Has 4 params, but very often param 2 - 4 is Player (so: Victim, player, player, player);
            }
        );*/ // Works but not super helpful, can't prevent KillMoves like that

		/*static FunctionHook<void(uintptr_t* param_1, uintptr_t* param_2, uintptr_t param_3, uintptr_t param_4)>CrimeHook (        
			REL::RelocationID(36430, 37425), 
			[](auto* param_1, auto* param_2, auto param_3, auto param_4) {
				log::info("Crime Hook");
				log::info("Param 1: {}", GetRawName(param_1));
				log::info("Param 2: {}", GetRawName(param_2));
				log::info("Param 3: {}", GetRawName(param_2));
				if (param_4) {
					log::info("Param 4: {}", GetRawName(param_2));
				}
				return CrimeHook(param_1, param_2, param_3, param_4);   // Has 4 params, but very often param 2 - 4 is Player (so: Victim, player, player, player);
            }
        );*/

		/*static FunctionHook<void(Actor* param_1, float param_2, Actor* param_3)>StaggerHook (        
			REL::RelocationID(36700, 37710), 
			[](auto* param_1, auto param_2, auto* param_3) {
				log::info("Stagger Hook");
				log::info("Param 1: {}", param_1->GetDisplayFullName());
				log::info("Param 2: {}", param_2);
				log::info("Param 3: {}", param_1->GetDisplayFullName());
				return StaggerHook(param_1, param_2, param_3);
            }
        );*/ // Works just fine

		/*static FunctionHook<float(Actor* param_1)>Actor_sub_140627930 (   // supposedly affects Weapon Damage                 
			REL::RelocationID(37650, 37650), // works only with direct weapon hits and fist attacks (physical non-arrow damage)
			[](auto* param_1) {
				//37650
				float result = Actor_sub_140627930(param_1);
				log::info("Actor_sub_140627930 Hooked, result: {}", result);
				return result;
            }
        );*/


		/*static CallHook<float(Actor* param_1)>CalculateCurrentHitTargetForWeaponSwing_140629090 (   // supposedly affects Weapon Damage                 
			REL::RelocationID(37674, 37674), REL::Relocate(0x2DD, 0x2DD),
			// sub_140627930 -> this function
			// 37674
			// 0x140627c0d - 0x140627930 = 0x2DD
			[](auto* param_1) {
				float result = CalculateCurrentHitTargetForWeaponSwing_140629090(param_1);
				log::info("CalculateCurrentHitTargetForWeaponSwing_140629090 Hooked, result: {}", result);
				return result;
            }
        );*/
		

		/*static CallHook<float(TESObjectREFR* param_1)>FUN_14080d560(        // rarely called             
			REL::RelocationID(48146, 48146), REL::Relocate(0x10D, 0x10D),
			// 48146
			// 0x14080d66d - 0x14080d560 = 0x10D
			//  AE
			// 
			// FUN_140838890 ???
			[](auto* param_1) {
				float result = FUN_14080d560(param_1);
				log::info("(0) FUN_14080d560 Hooked");
				return result * 10;
            }
        );
		//^ Hook 0 (Not taking [jmp] and [jbo32] hooks into account at the beginning)

		static CallHook<float(TESObjectREFR* param_1)>sub_1407BF260(                     
			REL::RelocationID(46074, 46074), REL::Relocate(0x7E, 0x7E),
			// 46074
			// 0x1407bf2de - 0x1407BF260 = 0x7E
			[](auto* param_1) {
				float result = sub_1407BF260(param_1);
				log::info("(1) sub_1407BF260 Hooked");
				return result * 10;
            }
        );
		//^ Hook 1
		static CallHook<float(TESObjectREFR* param_1)>sub_1407BF0C0(
			REL::RelocationID(46072, 46072), REL::Relocate(0x7D, 0x7D),
			// 46072
			// 0x1407bf13d - 0x1407BF0C0 = 0x7D
			[](auto* param_1) {
				float result = sub_1407BF0C0(param_1);
				log::info("(2) sub_1407BF0C0 Hooked");
				return result * 10;
            }
        );
		//^ Hook 2
/////////////////////////////////////////////////////////////////////////////////////////////////////

		static CallHook<float(TESObjectREFR* param_1)>sub_1407BEB90( // Uses GetScale() 2 times
			REL::RelocationID(46068, 46068), REL::Relocate(0x127, 0x127),
			// 46068
			// 0x1407becb7 - 0x1407BEB90 = 0x127
			[](auto* param_1) {
				float result = sub_1407BEB90(param_1);
				log::info("(3 - 1) sub_1407BEB90_1 Hooked"); // not sure what they do, seem to be called rarely
				return result;
            }
        );
		//^ Hook 3 ( Uses it 2 times)

		static CallHook<float(TESObjectREFR* param_1)>sub_1407BEB90_2( // Uses GetScale() 2 times
			// 46068
			// 0x1407bebce - 0x1407BEB90 = 0x3E
			REL::RelocationID(46068, 46068), REL::Relocate(0x3E, 0x3E),
			[](auto* param_1) {
				float result = sub_1407BEB90_2(param_1);
				log::info("(3 - 2) sub_1407BEB90_2 Hooked");
				return result;
            }
        );
		//^ Hook 4 ( Second part of hook above)

/////////////////////////////////////////////////////////////////////////////////////////////////////



		/*static CallHook<float(TESObjectREFR* param_1)>sub_1407BAB40(     // Seems to work after hitting someone/when we're detected. Disabling hostile actor stops prints.
			REL::RelocationID(46018, 46018), REL::Relocate(0x15, 0x15),   // USED INSIDE MOVEMENT.CPP
			// 46018
			// 0x1407bab55 - 0x1407BAB40 = 0x15
			[](auto* param_1) {
				float result = sub_1407BAB40(param_1);
				//log::info("(5) sub_1407BAB40 Hooked");	
				return result * 10;
            }
        ); 

		//^ Hook 5


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------
		static CallHook<float(TESObjectREFR* param_1)>sub_1407BA9C0(     // Not sure that it works at all
			REL::RelocationID(46015, 46015), REL::Relocate(0x9C, 0x9C),
			// 46015
			// 0x1407baa5c - 0x1407BA9C0 = 0x9C
			[](auto* param_1) {
				float result = sub_1407BA9C0(param_1);
				log::info("(6 - 1) sub_1407BA9C0_1 Hooked");
				return result;
            }
        );
		//^ Hook 6 
		static CallHook<float(TESObjectREFR* param_1)>sub_1407BA9C0_2(    // Not sure that it works at all
			REL::RelocationID(46015, 46015), REL::Relocate(0x57, 0x57),
			[](auto* param_1) {
				// 46015
				// 0x1407baa17 - 0x1407BA9C0 = 0x57
				float result = sub_1407BA9C0_2(param_1);
				log::info("(6 - 2) sub_1407BA9C0 Hooked");
				return result;
            }
        );
		//^ Hook 7 
//-----------------------------------------------------------------------------------------------------------		

////////////////////////////////////////////////////////////////////////////////////////////////////

	
		static CallHook<float(TESObjectREFR* param_1)>sub_1407BA910(        
			REL::RelocationID(46014, 46014), REL::Relocate(0x94, 0x94),
			[](auto* param_1) {
				// 46014
				// 0x1407ba9a4 - 0x1407BA910 = 0x94
				float result = sub_1407BA910(param_1);
				log::info("(8) sub_1407BA910 Hooked");
				return result;
            }
        );
		//^ Hook 8
		static CallHook<float(TESObjectREFR* param_1)>sub_1407BA8B0(           
			REL::RelocationID(46013, 46013), REL::Relocate(0x44, 0x44),
			[](auto* param_1) {
				// 46013
				// 0x1407ba8f4 - 0x1407BA8B0 = 0x44
				float result = sub_1407BA8B0(param_1);
				log::info("(9) sub_1407BA8B0 Hooked");
				return result;
            }
        );
		//^ Hook 9

		static CallHook<float(TESObjectREFR* param_1)>FUN_140760070(    
			REL::RelocationID(43242, 43242), REL::Relocate(0x54, 0x54),
			[](auto* param_1) {
				// 43242
				// 0x1407600c4 - 0x140760070 = 0x54
				float result = FUN_140760070(param_1);
				log::info("(10) FUN_140760070 Hooked");
				return result;
            }
        ); 
		
		//^ Hook 10 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		static CallHook<float(TESObjectREFR* param_1)>FUN_14075ffa0(      
			REL::RelocationID(43241, 43241), REL::Relocate(0x7E, 0x7E),
			[](auto* param_1) {
				// 43241  V
				// 0x14076001e - 0x14075ffa0 = 0x7E
				log::info("(11) FUN_14075ffa0 Hooked");
				float result = FUN_14075ffa0(param_1);
				return result;
            }
        );
		//^ Hook 11


		static CallHook<float(TESObjectREFR* param_1)>Handle_140723320(      // Something jump handle related 
			REL::RelocationID(41811, 41811), REL::Relocate(0x4D, 0x4D),
			[](auto* param_1) {
				// 41811 V
				// 0x14072336d - 0x140723320 = 0x4D
				float result = Handle_140723320(param_1);
				log::info("(12) Handle_140723320 Hooked");
				return result;
            }
        );
		//^ Hook 12*/


		/*static CallHook<float(TESObjectREFR* param_1)>FUN_14071b230(  // something bone related
			REL::RelocationID(41683, 41683), REL::Relocate(0x31, 0x31), // Affects Animation speed of: Walk Speed, Sneak Speed
			[](auto* param_1) {
				// 41683 V
				// 0x14071b261 - 0x14071b230 = 0x31
				log::info("(13) FUN_14071b230 Hooked");
				float result = FUN_14071b230(param_1);
				return result;
            }
        );
		//^ Hook 13

		static CallHook<float(TESObjectREFR* param_1)>FUN_1406b0a00(  // bone stuff again?
			REL::RelocationID(39540, 39540), REL::Relocate(0xE0, 0xE0),
			[](auto* param_1) {
				// 39540 V
				// 0x1406b0aab - 0x1406b0a00 = 0xAB
				log::info("(14) FUN_1406b0a00 Hooked");
				float result = FUN_1406b0a00(param_1);
				return result;
            }
        );
		//^ Hook 14

		static CallHook<float(TESObjectREFR* param_1)>sub_1406AA410(
			REL::RelocationID(39477, 39477), REL::Relocate(0xE0, 0xE0),
			[](auto* param_1) {
				// 39477  V
				// 0x1406aa4d1 - 0x1406AA410 = 0xC1
				log::info("(15) sub_1406AA410 Hooked");
				float result = sub_1406AA410(param_1);
				return result;
            }
        );

		//^ Hook 15

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		static CallHook<float(TESObjectREFR* param_1)>FUN_14065c4a0(
			REL::RelocationID(38564, 38564), REL::Relocate(0xB2, 0xB2),
			[](auto* param_1) {
				// 38564
				// 0x14065c552 - 0x14065c4a0 = 0xB2
				float result = FUN_14065c4a0(param_1);
				log::info("(16) FUN_14065c4a0 Hooked");
				return result;
            }
        );
		//^ Hook 16

		static CallHook<float(TESObjectREFR* param_1)>sub_14062E390(
			REL::RelocationID(37752, 37752), REL::Relocate(0x143, 0x143),
			[](auto* param_1) {
				// 37752
				// 0x14062e4d3 - 0x14062E390 = 0x143
				float result = sub_14062E390(param_1);
				log::info("(17) sub_14062E390 Hooked");
				return result;
            }
        );
		//^ Hook 17

		/*static CallHook<float(TESObjectREFR* param_1)>sub_140623F10( // Seems to be called on attacks. 
			REL::RelocationID(37588, 37588), REL::Relocate(0x6B, 0x6B), // Supposedly moves invisible "Hitbox" zone for weapons more forward or something
			[](auto* param_1) {                                         // Not sure.
				// 37588
				// 0x140623f7b - 0x140623F10 = 0x6B
				float result = sub_140623F10(param_1) * 10.0;
				log::info("(18) sub_140623F10 Hooked");
				return result;
            }
        );
		//^ Hook 18

		static CallHook<float(TESObjectREFR* param_1)>sub_140619040( // No clue what it does, called rarely
			REL::RelocationID(37323, 37323), REL::Relocate(0x17E, 0x17E), 
			[](auto* param_1) {
				// 37323
				// 0x1406191be - 0x140619040 = 0x17E
				float result = sub_140619040(param_1) * 10.0;
				log::info("(19) sub_140619040 Hooked");
				return result;
            }
        );
		//^ Hook 19

		/*static CallHook<float(TESObjectREFR* param_1)>Alter_Headtracking(  /// HEADTRACKING HOOK!
			REL::RelocationID(37129, 37129), REL::Relocate(0x24, 0x24),
			[](auto* param_1) {
				// 37129
				// 0x140615054 - 0x140615030 = 0x24
				// FUN_140615030
				float result = Alter_Headtracking(param_1);
				float Alter = affect_by_scale(param_1, result);
				//log::info("(20) Alter_Headtracking Hooked");
				return Alter;
            }
        );*/
		//^ Hook 20
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/*static CallHook<float(TESObjectREFR* param_1)>Scale_AlterMovementSpeed(  // Movement speed alteration
			REL::RelocationID(37013, 37013), REL::Relocate(0x1A, 0x1A),            // USED INSIDE MOVEMENT.CPP
			[](auto* param_1) {
				// sub_14060EEF0 : 37013
				// 0x14060ef0a - 0x14060EEF0 = 0x1A
				float result = 1.0; // force it to 1.0. We DON'T want the SetScale() to affect it.
				//log::info("(21) Scale_AlterMovementSpeed Hooked");
				return result;
            }
        );
		//^ Hook 21

		static CallHook<float(TESObjectREFR* param_1)>Func7_140609D50( // Something AnimationGraphManager related
			REL::RelocationID(36957, 36957), REL::Relocate(0xBD, 0xBD),
			[](auto* param_1) {
				// 36957
				// 0x140609e0d - 0x140609D50 = 0xBD
				float result = Func7_140609D50(param_1);
				log::info("(22) Func7_140609D50 Hooked");
				return result;
            }
        );
		//^ Hook 22

		/*static CallHook<float(TESObjectREFR* param_1)>GetEyeHeight_140601E40(  // Get Eye Height, rarely called
			REL::RelocationID(36845, 36845), REL::Relocate(0x71, 0x71),         // USED INSIDE HEADTRACKING.cpp
			[](auto* param_1) {
				// 36845
				// 0x140601eb1 - 0x140601E40 = 0x71
				float result = GetEyeHeight_140601E40(param_1) * 10.0;
				log::info("(23) GetEyeHeight_140601E40 Hooked");
				return result;
            }
        );
		//^ Hook 23

		static CallHook<float(TESObjectREFR* param_1)>sub_140601D80(
			REL::RelocationID(36844, 36844), REL::Relocate(0x47, 0x47),
			[](auto* param_1) {
				// 36844
				// 0x140601dc7 - 0x140601D80 = 0x47
				float result = sub_140601D80(param_1);
				log::info("(24) sub_140601D80 Hooked");
				return result;
            }
        );
		//^ Hook 24

		static CallHook<float(TESObjectREFR* param_1)>sub_140601CD0( // not sure what it is, rarely called
			REL::RelocationID(36843, 36843), REL::Relocate(0x44, 0x44),
			[](auto* param_1) {
				// 36843
				// 0x140601d14 - 0x140601CD0 = 0x44
				float result = sub_140601CD0(param_1);
				log::info("(25) sub_140601CD0 Hooked");
				return result;
            }
        );
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//^ Hook 25

		static CallHook<float(TESObjectREFR* param_1)>sub_140601C20( // Seen in logs non-stop but no clue what it does
			REL::RelocationID(36842, 36842), REL::Relocate(0x44, 0x44),
			[](auto* param_1) {
				// 36842
				// 0x140601c64 - 0x140601C20 = 0x44
				float result = sub_140601C20(param_1) * 30;
				//log::info("(26) sub_140601C20 Hooked");
				return result;
            }
        );
		//^ Hook 26

		static CallHook<float(TESObjectREFR* param_1)>sub_140601B50( // occasionally seen in logs but no clue what it does
			REL::RelocationID(36841, 36841), REL::Relocate(0x58, 0x58),
			[](auto* param_1) {
				// 36841
                // 0x140601ba8 - 0x140601B50 = 0x58
				float result = sub_140601B50(param_1);
				log::info("(27) sub_140601B50 Hooked");
				return result;
            }
        );
		//^ Hook 27

		static CallHook<float(TESObjectREFR* param_1)>UndefinedFunction_1406018a0(
			REL::RelocationID(36838, 36838), REL::Relocate(0x62, 0x62),
			[](auto* param_1) {
				// 36838
				// 0x140601902 - 0x1406018a0 = 0x62
				float result = UndefinedFunction_1406018a0(param_1) * 50;
				log::info("(28) UndefinedFunction_1406018a0 Hooked");
				return result;
            }
        );
		//^ Hook 28

		static CallHook<float(TESObjectREFR* param_1)>sub_1405F1DE0(   // something player related
			REL::RelocationID(36628, 36628), REL::Relocate(0x16C, 0x16C),
			[](auto* param_1) {
				// 36628
				// 0x1405f1f4c - 0x1405F1DE0 = 0x16C
				float result = sub_1405F1DE0(param_1);
				log::info("(29) sub_1405F1DE0 Hooked");
				return result;
            }
        );
		//^ Hook 29
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------

		/*static CallHook<float(TESObjectREFR* param_1)>sub_1405ED870_1(   // something player related
			REL::RelocationID(36595, 36595), REL::Relocate(0xDE, 0xDE),
			[](auto* param_1) {
				// 36595
				// 0x1405ed94e - 0x1405ED870 = 0xDE
				float result = sub_1405ED870_1(param_1);
				log::info("(30 - 1) sub_1405ED870 Hooked");
				return result;
            }
        );
		//^ Hook 30

		static CallHook<float(TESObjectREFR* param_1)>sub_1405ED870_2(   // something player related
			REL::RelocationID(36595, 36595), REL::Relocate(0xD3, 0xD3),
			[](auto* param_1) {
				// 36595
				// 0x1405ed943 - 0x1405ED870 = 0xD3
				float result = sub_1405ED870_2(param_1);
				log::info("(30 - 2) sub_1405ED870 Hooked");
				return result;
            }
        );
		//^ Hook 31
//-----------------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		static CallHook<float(TESObjectREFR* param_1)>sub_1405ED750(  // called often, no clue what it does
			REL::RelocationID(36594, 36594), REL::Relocate(0xB7, 0xB7),
			[](auto* param_1) {
				// 36594
				// 0x1405ed807 - 0x1405ED750 = 0xB7
				float result = sub_1405ED750(param_1) * 10.0;
				//log::info("(32) sub_1405ED750 Hooked");
				return result;
            }
        );
		//^ Hook 32

		static CallHook<float(TESObjectREFR* param_1)>sub_1405ED6D0( 
			REL::RelocationID(36593, 36593), REL::Relocate(0x53, 0x53),
			[](auto* param_1) {
				// 36593
				// 0x1405ed723 - 0x1405ED6D0 = 0x53
				float result = sub_1405ED6D0(param_1) * 10.0;
				log::info("(33) sub_1405ED6D0 Hooked");
				return result;
            }
        );
		//^ Hook 33

		static CallHook<float(TESObjectREFR* param_1)>sub_1405ED680(  // may crash, something weight related? (Stealth?)
			REL::RelocationID(36592, 36592), REL::Relocate(0x34, 0x34),
			[](auto* param_1) {
				// 36592
				// 0x1405ed6b4 - 0x1405ED680 = 0x34
				float result = sub_1405ED680(param_1) * 10.0;
				log::info("(34) sub_1405ED680 Hooked");
				return result;
            }
        );
		//^ Hook 34
//-----------------------------------------------------------------------------------------------------------

		static CallHook<float(TESObjectREFR* param_1)>GetScaledBoundSize_1405E1300(  // Called often but no clue what it does.
			REL::RelocationID(36448, 36448), REL::Relocate(0xDB, 0xDB),
			// 36448
			// 0x1405e13db - 0x1405E1300 = 0xDB
			[](auto* param_1) {
				float result = GetScaledBoundSize_1405E1300(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(35 - 1) GetScaledBoundSize_1405E1300 Hooked");
				return Alter;
            }
        );
		//^ Hook 35

		static CallHook<float(TESObjectREFR* param_1)>GetScaledBoundSize_1405E1300_2(  // Probably affects attack (?) distance
			REL::RelocationID(36448, 36448), REL::Relocate(0x75, 0x75),
			// 36448
			// 0x1405e1375 - 0x1405E1300 = 0x75
			[](auto* param_1) {
				float result = GetScaledBoundSize_1405E1300_2(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(35 - 2) GetScaledBoundSize_1405E1300 Hooked");
				return Alter;
            }
        );
		//^ Hook 36
//-----------------------------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		static CallHook<float(TESObjectREFR* param_1)>GetFlightRadius( 
			REL::RelocationID(36445, 36445), REL::Relocate(0x1D, 0x1D),
			// 1405e1200 : 36445
			// 0x1405e121d - 0x1405e1200 = 0x1D
			[](auto* param_1) {
				float result = GetFlightRadius(param_1);
				log::info("(37) GetFlightRadius Hooked");
				return result;
            }
        );
		//^ Hook 37

		static CallHook<float(TESObjectREFR* param_1)>sub_1405D87F0(   // something player related
			REL::RelocationID(36365, 36365), REL::Relocate(0x1FD, 0x1FD),
			// 36365
			// 0x1405d89ed - 0x1405D87F0 = 0x1FD
			[](auto* param_1) {
				float result = sub_1405D87F0(param_1);
				log::info("(38) sub_1405D87F0 Hooked");
				return result;
            }
        );
		//^ Hook 38

		/*static CallHook<float(TESObjectREFR* param_1)>Jump_1405D1F80(   // WE ALREADY HOOK THAT!
			REL::RelocationID(36758, 36758), REL::Relocate(0xE0, 0xE0),
			// 1405d89ed
			[](auto* param_1) {
				float result = Jump_1405D1F80(param_1);
				log::info("(39) Jump_1405D1F80 Hooked");
				return result;
            }
        );
		//^ Hook 39
		
		static CallHook<float(TESObjectREFR* param_1)>FUN_1405513a0(   // Possible headtracking? angle stuff or something
			REL::RelocationID(33678, 33678), REL::Relocate(0x50, 0x50),
			// 33678
			// 0x1405513f0 - 0x1405513a0 = 0x50
			[](auto* param_1) {
				float result = FUN_1405513a0(param_1);
				log::info("(40) FUN_1405513a0 Hooked");
				return result;
            }
        );
		//^ Hook 40

		static CallHook<float(TESObjectREFR* param_1)>UndefinedFunction_14054c740( 
			REL::RelocationID(33628, 33628), REL::Relocate(0x90, 0x90),
			// 33628
			// 0x14054c7d0 - 0x14054c740 = 0x90
			[](auto* param_1) {
				float result = UndefinedFunction_14054c740(param_1) * 10.0;
				log::info("(41) UndefinedFunction_14054c740 Hooked");
				return result;
            }
        );
		//^ Hook 41	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------

		/*static CallHook<float(TESObjectREFR* param_1)>sub_1404E6B30_1(
			REL::RelocationID(31951, 31951), REL::Relocate(0x1F9, 0x1F9),
			[](auto* param_1) {
				// 31951
				// 0x1404e6d29 - 0x1404E6B30 = 0x1F9
				float result = sub_1404E6B30_1(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(42 - 1) sub_1404E6B30 Hooked");
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
				log::info("(42 - 2) sub_1404E6B30 Hooked");
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
				log::info("(42 - 3) sub_1404E6B30 Hooked");
				return Alter;
            }
        );
		//^ Hook 44
//-----------------------------------------------------------------------------------------------------------		
/////////////////////////////////////////////////////////////////////////////////////////////////////

		static CallHook<float(TESObjectREFR* param_1)>sub_1404E6360(     // Called very often, not sure what it does. 
			REL::RelocationID(31949, 31949), REL::Relocate(0x55A, 0x55A), // Maybe attack distance of NPC's?
			// 31949
			// 0x1404e68ba - 0x1404E6360 = 0x55A
			[](auto* param_1) {
				float result = sub_1404E6360(param_1);
				float Adjust = 1.0;//affect_by_scale(param_1, result);
				//log::info("(45) Hooked");
				return Adjust;
            }
        );
		//^ Hook 45

/////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------
		static CallHook<float(TESObjectREFR* param_1)>sub_140474420(   
			REL::RelocationID(29837, 29837), REL::Relocate(0x1FA, 0x1FA),
			// 29837
			// 0x14047461a - 0x140474420 = 0x1FA
			[](auto* param_1) {
				float result = sub_140474420(param_1);
				log::info("(46 - 1) sub_140474420 Hooked");
				float Adjust = affect_by_scale(param_1, result);
				return Adjust;
            }
        );
		//^ Hook 46

		static CallHook<float(TESObjectREFR* param_1)>sub_140474420_2(   
			REL::RelocationID(29837, 29837), REL::Relocate(0x150, 0x150),
			// 29837
			// 0x140474570 - 0x140474420 = 0x150
			[](auto* param_1) {
				float result = sub_140474420_2(param_1);
				log::info("(46 - 2) sub_140474420 Hooked");
				float Adjust = affect_by_scale(param_1, result);
				return Adjust;
            }
        );
		//^ Hook 47
//-----------------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		static CallHook<float(TESObjectREFR* param_1)>sub_140359960(   
			REL::RelocationID(24107, 24107), REL::Relocate(0x135, 0x135),
			// 24107
			// 0x140359a95 - 0x140359960 = 0x135
			[](auto* param_1) {
				float result = sub_140359960(param_1);
				log::info("(48) sub_140359960 Hooked");
				return result;
            }
        );
		//^ Hook 48

		static CallHook<float(TESObjectREFR* param_1)>ModScale_1402F4710(
			REL::RelocationID(21586, 21586), REL::Relocate(0x5D, 0x5D),
			// 21586
			// 0x1402f476d - 0x1402F4710 = 0x5D
			[](auto* param_1) {
				float result = ModScale_1402F4710(param_1);
				log::info("(49) ModScale_1402F4710 Hooked");
				return result;
            }
        );
		//^ Hook 49

		static CallHook<float(TESObjectREFR* param_1)>GetScale_1402D4EC0(
			REL::RelocationID(20971, 20971), REL::Relocate(0x58, 0x58),
			// 20971
			// 0x1402d4f18 - 0x1402D4EC0 = 0x58
			[](auto* param_1) {
				float result = GetScale_1402D4EC0(param_1);
				log::info("(50) GetScale_1402D4EC0 Hooked");
				return result;
            }
        );
		//^ Hook 50

		static CallHook<float(TESObjectREFR* param_1)>sub_1402AA350(        // NiNode*
			REL::RelocationID(19889, 19889), REL::Relocate(0xBC, 0xBC),
			// 19889
			// 0x1402aa40c - 0x1402AA350 = 0xBC
			[](auto* param_1) {
				float result = sub_1402AA350(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(51) sub_1402AA350 Hooked");
				return Alter;
            }
        );
		//^ Hook 51



		static CallHook<float(TESObjectREFR* param_1)>sub_1402A9740(        // Weird function
			REL::RelocationID(19866, 19866), REL::Relocate(0xAE, 0xAE),
			// 19866
			// 0x1402a97ee - 0x1402A9740 = 0xAE
			[](auto* param_1) {
				float result = sub_1402A9740(param_1) * 50.0;
				log::info("(52) sub_1402A9740 Hooked");
				return result;
            }
        );
		//^ Hook 52
	

		static CallHook<float(TESObjectREFR* param_1)>InitializeAfterAllFormsAreReadFromFile_1402850E0(        // Probably dangerous to alter
			REL::RelocationID(19105, 19105), REL::Relocate(0x305, 0x305),
			// 19105
			// 0x1402853e5 - 0x1402850E0 = 0x305
			[](auto* param_1) {
				float result = InitializeAfterAllFormsAreReadFromFile_1402850E0(param_1);
				log::info("(53) InitializeAfterAllFormsAreReadFromFile_1402850E0 Hooked");
				return result;
            }
        );
		//^ Hook 53

		static CallHook<float(TESObjectREFR* param_1)>FUN_140278070(          // seems to be applied on save load
			REL::RelocationID(18836, 18836), REL::Relocate(0x20E, 0x20E),
			// 18836
			// 0x14027827e - 0x140278070 = 0x20E
			[](auto* param_1) {
				float result = FUN_140278070(param_1) * 20;
				log::info("(54) FUN_140278070 Hooked");
				return result;
            }
        );
		//^ Hook 54

		static CallHook<float(TESObjectREFR* param_1)>sub_140239B20(       // supposedly applies on dealing damage
			REL::RelocationID(17807, 17807), REL::Relocate(0x95, 0x95),   // Called almost every frame which is weird, yet does nothing.
			// 17807
			// 0x140239bb5 - 0x140239B20 = 0x95
			[](auto* param_1) {
				float result = sub_140239B20(param_1) * 25;
				//log::info("(55) sub_140239B20 Hooked");
				return result;
            }
        );
		//^ Hook 55
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------
		/*static CallHook<float(TESObjectREFR* param_1)>Clone3D_140222410(       // Probably a bad idea to hook it in general
			REL::RelocationID(36758, 36758), REL::Relocate(0xE0, 0xE0),
			// 140222642
			[](auto* param_1) {
				float result = Clone3D_140222410(param_1);
				log::info("(56 - 1) Clone3D_140222410 Hooked");
				return result;
            }
        );
		//^ Hook 56

		static CallHook<float(TESObjectREFR* param_1)>Clone3D_140222410_2(       // Probably a bad idea to hook it in general
			REL::RelocationID(36758, 36758), REL::Relocate(0xE0, 0xE0),
			// 14022261d
			[](auto* param_1) {
				float result = Clone3D_140222410_2(param_1);
				log::info("(56 - 2) Clone3D_140222410 Hooked");
				return result;
            }
        );*/
		//^ Hook 57
//-----------------------------------------------------------------------------------------------------------		
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------
		/*static CallHook<float(TESObjectREFR* param_1)>LoadGraphics_140220DD0(       
			REL::RelocationID(17260, 17260), REL::Relocate(0x1FC, 0x1FC),
			// 17260
			// 0x140220fcc - 0x140220DD0 = 0x1FC
			[](auto* param_1) {
				float result = LoadGraphics_140220DD0(param_1);
				log::info("(58 - 1) LoadGraphics_140220DD0 Hooked");
				return result;
            }
        );
		//^ Hook 58

		static CallHook<float(TESObjectREFR* param_1)>LoadGraphics_140220DD0_2(       
			REL::RelocationID(17260, 17260), REL::Relocate(0x1B9, 0x1B9),
			// 17260
			// 0x140220f89 - 0x140220DD0 = 0x1B9
			[](auto* param_1) {
				float result = LoadGraphics_140220DD0_2(param_1);
				log::info("(58 - 2) LoadGraphics_140220DD0 Hooked");
				return result;
            }
        );
		//^ Hook 59
//-----------------------------------------------------------------------------------------------------------		

///////////////////////////////////////////////////////////////////////////////////////////////////// 

		static CallHook<float(TESObjectREFR* param_1)>sub_140220CA0(        // seems to be called during save load only
			REL::RelocationID(17259, 17259), REL::Relocate(0x60, 0x60),
			// 17259
			// 0x140220d00 - 0x140220CA0 = 0x60
			[](auto* param_1) {
				float result = sub_140220CA0(param_1) * 25;
				log::info("(60) sub_140220CA0");
				return result;
            }
        );
		//^ Hook 60

		static CallHook<float(TESObjectREFR* param_1)>sub_140220A30(       // something TESForm related, probably damage
			REL::RelocationID(17256, 17256), REL::Relocate(0x53, 0x53),    //  leads to crashes on save load if we alter it.
			// 17256
			// 0x140220a83 - 0x140220A30 = 0x53
			[](auto* param_1) {
				float result = sub_140220A30(param_1);
				//log::info("(61) sub_140220A30");
				return result;
            }
        );
		//^ Hook 61
		

		static CallHook<float(TESObjectREFR* param_1)>UndefinedFunction_14021ecb0(
			REL::RelocationID(17206, 17206), REL::Relocate(0xF9, 0xF9),
			// 17206
			// 0x14021eda9 - 0x14021ecb0 = 0xF9
			[](auto* param_1) {
				float result = UndefinedFunction_14021ecb0(param_1) * 50;
				log::info("(62) UndefinedFunction_14021ecb0");
				return result;
            }
        );
		//^ Hook 62

		static CallHook<float(TESObjectREFR* param_1)>FUN_1401aa570(
			REL::RelocationID(14970, 14970), REL::Relocate(0x5B, 0x5B),
			// 14970
			// 0x1401aa5cb - 0x1401aa570 = 0x5B
			[](auto* param_1) {
				float result = FUN_1401aa570(param_1) * 10.0;
				log::info("(63) FUN_1401aa570 hooked");
				return result;
            }
        );
		//^ Hook 63

		static CallHook<float(TESObjectREFR* param_1)>sub_1401844D0( // Not damage
			REL::RelocationID(14062, 14062), REL::Relocate(0x7B, 0x7B),
			// 14062
			// 0x14018454b - 0x1401844D0 = 0x7B
			[](auto* param_1) {
				float result = sub_1401844D0(param_1) * 25.0;
				log::info("(64) sub_1401844D0 hooked");
				return result;
            }
        );
		//^ Hook 64

		static CallHook<float(TESObjectREFR* param_1)>sub_1401834F0( // Not damage
			REL::RelocationID(14059, 14059), REL::Relocate(0x8FD, 0x8FD),
			// 14059
			// 0x140183ded - 0x1401834F0 = 0x8FD
			[](auto* param_1) {
				float result = sub_1401834F0(param_1) * 10.0;
				log::info("(65) sub_1401834F0 hooked");
				return result;
            }
        );
		//^ Hook 65

		static CallHook<float(TESObjectREFR* param_1)>UndefinedFunction_140145840( // Not damage
			REL::RelocationID(12872, 12872), REL::Relocate(0x86, 0x86),
			// 12872
			// 0x1401458c6 - 0x140145840 = 0x86
			[](auto* param_1) {
				float result = UndefinedFunction_140145840(param_1) * 10.0;
				log::info("(66) UndefinedFunction_140145840");
				return result;
            }
        );
		//^ Hook 66


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		/*static CallHook<float(Actor* param_1, uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uintptr_t param_5,
			uintptr_t param_6, uintptr_t param_7, uintptr_t param_8, uintptr_t param_9, uintptr_t param_10)>CalculateDetection_1405FD870(
			REL::RelocationID(36758, 36758), REL::Relocate(0xE0, 0xE0),
			[](auto* param_1, auto param_2, auto param_3, auto param_4, auto param_5, auto param_6, auto param_7, auto param_8, auto param_9, auto param_10) {
				float result = CalculateDetection_1405FD870(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
				log::info("Hook Result for {} is {}", param_1->GetDisplayFullName(), result);
				return result;
            }
        );*/

		/*static FunctionHook<void(TESObjectREFR* ref, float X)>Skyrim_SetAngleX(  
            REL::RelocationID(19360, 19360), // 19360 = 140296680
            [](auto* ref, auto X) {
				Actor* actor = skyrim_cast<Actor*>(ref);
				if (actor) {
					if (actor->formID != 0x14 && IsTeammate(actor)) {
						//X = random;
						log::info("Value post: {}", X);
					}
				}
                return Skyrim_SetAngleX(ref, X);
				// rotation is capped between 1.57 and - 1.57 based on prints
            }
        );

		static FunctionHook<void(Actor* ref, float X)>Skyrim_SetRotationX( 
            REL::RelocationID(36602, 37610),
            [](auto* ref, auto X) {
				if (ref->formID != 0x14 && IsTeammate(ref)) {
					log::info("- SetRotation X is called for {}", ref->GetDisplayFullName());
					log::info("- Value: {}", X);
				}
				
                return Skyrim_SetRotationX(ref, X);
            }
        );*/

		/*static FunctionHook<float(NiCamera* camera)> Skyrim_Camera(  // camera hook works just fine that way
            REL::RelocationID(69271, 70641),
            [](auto* camera) {
				//log::info("Camera hook is running");
				float result = Skyrim_Camera(camera);
				log::info("Hook Result: {}", result);
                return result;
            }
        );*/

		/*static FunctionHook<NiPoint3(PlayerCamera* camera)> Skyrim_FactorCameraOffset(  // camera hook works just fine that way
            REL::RelocationID(49866, 50799),
            [](auto* camera) {
				//log::info("Camera hook is running");
				NiPoint3 result = Skyrim_FactorCameraOffset(camera);
				log::info("Hook Result: {}", Vector2Str(result));
				result.x += 5.0;
				result *= camera_getplayersize();
				log::info("Hook Result altered: {}", Vector2Str(result));
                return result;
            }
        );*/

		/*static FunctionHook<NiPoint3(PlayerCamera* camera)> Skyrim_UpdateCamera(  // camera hook works just fine that way
            REL::RelocationID(49852, 49852), // PlayerCamera::Update_14084AB90
            [](auto* camera) {
				//log::info("Camera hook is running");
				NiPoint3 result = Skyrim_UpdateCamera(camera);
				log::info("Hook Result: {}", Vector2Str(result));
				result *= camera_getplayersize();
				log::info("Hook Result Post: {}", Vector2Str(result));
                return result;
            }
        );/*

		// Yet the one below CTD's. Sigh.
		/*static CallHook<float(const NiCamera* camera)> Skyrim_Camera_posX(RELOCATION_ID(69271, 70641),  REL::Relocate(0x11, 0x11), // ctd, ctd and ctd.
		[](const NiCamera* camera) { // 0x140C66710 - 0x140c66b70 (fVar18) = 0x11 . No AE rel
			log::info("Trying to hook camera");
			//auto result = Skyrim_Camera_posX(camera);
			log::info("Hooked camera");
			double result = 0.0;
			log::info("Pos X is 0");
			return result;
		});*/

		/*static CallHook<float(uintptr_t* cam)> Skyrim_Camera_posY(RELOCATION_ID(69271, 70641),  REL::Relocate(0x64, 0x64),
		[](auto* cam) { // 0x140C66710 - 0x140c66774 (fVar14) = 0x64 . No AE rel.
			if (cam) {
				float result = Skyrim_Camera_posY(cam);
				log::info("Pos Y: {}", result);
				return result;
			}
		});

		static CallHook<float(uintptr_t* cam)> Skyrim_Camera_posZ(RELOCATION_ID(69271, 70641),  REL::Relocate(0x19, 0x19),
		[](auto* cam) { // 0x140C66710 - 0x140c66729 (fVar15) = 0x19 . No AE rel.
			if (cam) {
				float result = Skyrim_Camera_posZ(cam);
				//log::info("Pos Z: {}", result);
				return result;
			}
		});*/

		// AE 1402bc7c3
		// SE 1402aa40c
		//
		// Seems to be unused
		//     fVar1 = TESObjectREFR::GetScale_14028CC60(param_2);
		//     (param_3->LocalTransform_48).Scale_30 = (float)((uint)(fVar1 * fVar2) & 0x7fffffff);
		//
		//     It sets the NiNode scale to match that of an GetScale Call
		//
		// static CallHook<float(TESObjectREFR*)> GetScaleHook1(RELOCATION_ID(19889, 20296),  REL::Relocate(0xbc, 0xbc, 0x493),
		// [](auto* self) {
		//     float result = GetScaleHook1(self);
		//     Actor* actor = skyrim_cast<Actor*>(self);
		//     if (actor) {
		//       float scale = get_visual_scale(actor);
		//       if (scale > 1e-4) {
		//         log::info("Scale Hook: {} for {}", scale, actor->GetDisplayFullName());
		//         result *= scale;
		//       }
		//     }
		//     return result;
		// });

		// SE: 140290bf8
		// Used during Set3D

		/*static FunctionHook<float(TESObjectREFR* ref)> Skyrim_GetScale_14028CC60(  // 19238 = 14028CC60 (SE), AE = ???
		    // Very unstable hook to say the least. Does same effect as SetScale() command (without fps cost)
			// Seems to multiply actor speed and causes chaos in-game overall
			// Actors fly and behave weirdly. The only positive effect of this hook is fixed Headtracking and possible Pathing improvements.
            REL::RelocationID(19238, 19238),
            [](auto* ref) {
                float result = Skyrim_GetScale_14028CC60(ref);
                Actor* actor = skyrim_cast<Actor*>(ref);
                if (actor) {
					float scale = get_visual_scale(actor);
                    result *= scale;
                    log::info("Scale Hook: {} for {}", scale, actor->GetDisplayFullName());
                }

                return result;
            }
        );*/

		/*static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4)> Skyrim_Pathing_140473120( 
            REL::RelocationID(29819, 29819),
            [](auto* param_1, auto param_2, auto param_3, auto param_4) {
                
				log::info("3120: Param 1: {}", GetRawName(param_1)); 
				log::info("3120: Param 2: {}", param_2); 
				log::info("3120: Param 3: {}", param_3); 
				log::info("3120: Param 4: {}", param_4); 

                return Skyrim_Pathing_140473120(param_1, param_2, param_3, param_4);
            }
        );

		static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4, uintptr_t param_5)> Skyrim_Pathing_140473490( 
            REL::RelocationID(29824, 29824),
            [](auto* param_1, auto param_2, auto param_3, auto param_4, auto param_5) {
                
				log::info("3490: Param 1: {}", GetRawName(param_1)); 
				log::info("3490: Param 2: {}", param_2); 
				log::info("3490: Param 3: {}", param_3); 
				log::info("3490: Param 4: {}", param_4); 
				log::info("3490: Param 5: {}", param_5); 

                return Skyrim_Pathing_140473490(param_1, param_2, param_3, param_4, param_5);
            }
        );

		static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4)> Skyrim_Pathing_140473200( 
            REL::RelocationID(29820, 29820),
            [](auto* param_1, auto param_2, auto param_3, auto param_4) {
                
				log::info("3200: Param 1: {}", GetRawName(param_1)); 
				log::info("3200: Param 2: {}", param_2); 
				log::info("3200: Param 3: {}", param_3); 
				log::info("3200: Param 4: {}", param_4); 

                return Skyrim_Pathing_140473200(param_1, param_2, param_3, param_4);
            }
        );*/

		/*static CallHook<float(NiNode* node)> Skyrim_NiNode(RELOCATION_ID(19889, 19889),  REL::Relocate(0xBC, 0xBC), // Prints nothing
		[](auto* node) {
		    float result = Skyrim_NiNode(node);
			log::info("Original Node Value: {}", result);
		    if (node) {
				float scale = 10.0;
				result *= scale;
				
				log::info("Node Value: {}", result);
		    }
		    return result;
		});*/

		/*static CallHook<float(TESObjectREFR* ref)> Skyrim_sub_140619040(RELOCATION_ID(37323, 37323),  REL::Relocate(0x17E, 0x17E), // Prints stuff but unsure what it does. 
		// Seems to be applied when new objects (actors?) are loaded into the scene, almost always reports negative value after * scale
		[](auto* ref) {
			// No idea what it does, rarely prints something.
		    float result = Skyrim_sub_140619040(ref);
			log::info("Original Ref Value: {}", result);
		    if (ref) {
				Actor* giant = skyrim_cast<Actor*>(ref);
				if (giant) {
					float scale = get_visual_scale(giant);
					result *= scale;
				}
				
				log::info("Ref New Scale: {}", result);
		    }
		    return result;
		});

		static CallHook<float(IAnimationGraphManagerHolder* graph)> Skyrim_AnimGraph_140609D50(RELOCATION_ID(36957, 36957),  REL::Relocate(0xBD, 0xBD),
		[](auto* graph) {
		    float result = Skyrim_AnimGraph_140609D50(graph);
			log::info("Original Graph Value: {}", result);
			Actor* giant = skyrim_cast<Actor*>(graph);
			if (giant) {
				if (giant->formID == 0x14 || IsTeammate(giant)) {
					float scale = get_visual_scale(giant);
					if (scale > 0.0) {
						result *= scale;
						log::info("Found Actor: {}, scale: {}", giant->GetDisplayFullName(), scale);
					}

				log::info("Graph New Scale: {}", result);
				}
			}
		    return result;
		});*/
	}
}
