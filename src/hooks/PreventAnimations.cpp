#include "hooks/PreventAnimations.hpp"
#include "data/transient.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"
#include "utils/debug.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    const float KillMove_Threshold = 1.15f;

	// Actions taht we want to prevent
	const auto DefaultSheathe = 			0x46BB2;
	const auto JumpRoot =					0x88302;
	const auto NonMountedDraw = 			0x1000992;
	const auto NonMountedForceEquip = 		0x1000993;
	const auto JumpStandingStart =        	0x884A2;   // 558242
	const auto JumpDirectionalStart =       0x884A3;   // 558243

	// Killmoves that we want to prevent
	const auto KillMoveFrontSideRoot =      0x24CD4;
	const auto KillMoveDragonToNPC =        0xC1F20;
	const auto KillMoveRootDragonFlight =   0xC9A1B;
	const auto KillMoveBackSideRoot =       0xE8458;
	const auto KillMoveFrontSideRoot00 =    0x100e8B;
	const auto KillMoveBackSideRoot00 =     0x100F16;

	bool PreventKillMove(FormID idle, ConditionCheckParams* params, Actor* performer, TESObjectREFR* victim) {
		// KillMoves
		
		bool KillMove = false;
		bool Block = false;

		switch (idle) {
			case KillMoveFrontSideRoot:
				KillMove = true;
			break;	
			case KillMoveDragonToNPC:
				KillMove = true;
			break;
			case KillMoveRootDragonFlight:
				KillMove = true;
			break;
			case KillMoveBackSideRoot:
				KillMove = true;
			break;
			case KillMoveFrontSideRoot00:
				KillMove = true;
			break;
			case KillMoveBackSideRoot00:
				KillMove = true;
			break;
		}

		if (KillMove) {
			log::info("Trying Killmove, seeking for Actors");
			if (victim) {
				log::info("KillMove: Performer: {}, Victim: {}", performer->GetDisplayFullName(), victim->GetDisplayFullName());
				Actor* victimref = skyrim_cast<Actor*>(victim);
				if (victimref) {
					float size_difference = GetSizeDifference(victimref, performer, SizeType::GiantessScale, true, false);
					log::info("Victimref found, size_difference: {}", size_difference);
					if (size_difference > KillMove_Threshold) {
						Block = true;
					}
				}
			}
		}

		return Block;
	}

	bool BlockAnimation(TESIdleForm* idle, ConditionCheckParams* params) {
		if (!idle) {
			return false;
		}

		auto Form = idle->formID;

		Actor* performer = params->actionRef->As<RE::Actor>();

		if (performer) { //  && performer->formID != 0x14
			log::info("Performer: {}", performer->GetDisplayFullName());

			if (PreventKillMove(Form, params, performer, params->targetRef)) {
				log::info("KILLMOVE PREVENTED");
				return true;
			}

			if (!IsGtsBusy(performer)) {
				return false;
			}

			switch (Form) {
				case DefaultSheathe:
					return true;
				break;	
				case JumpRoot:
					return true;
				break;	
				case NonMountedDraw:
					return true;
				break;	
				case NonMountedForceEquip:
					return true;
				break;	
				case JumpStandingStart:
					return true;	
				break;	
				case JumpDirectionalStart:
					return true;	
				break;
				return false;
			}
			return false;
		}
		return false;
	}
}

namespace Hooks {

	void Hook_PreventAnimations::Hook(Trampoline& trampoline) { 
        static CallHook<TESIdleForm*(TESIdleForm* a_this, ConditionCheckParams* params, void* unk3)>IdleFormHook (        
			REL::RelocationID(24068, 24571), REL::Relocate(0x5E, 0x53),
			// 24068 = 0x140358250 [SE] ; 0x140358250 - 0x1403582ae = 0x5E
			// 24571 = 0x14036ee00 [AE] ; 0x14036ee00 - 0x14036ee53 = 0x53
			[](TESIdleForm* a_this, ConditionCheckParams* params, void* unk3) {
				
				auto* result = IdleFormHook(a_this, params, unk3);

				if (a_this) {
					//log::info("Playing Idle: {}", a_this->animFileName); // prints Actors\Character\Behaviors\0_Master.hkx for example
					//log::info("Playing Idle ID: {}", a_this->formID);
					//log::info("Playing Idle Name: {}", a_this->animEventName);
					//log::info("Playing formEditorID: {}", a_this->formEditorID.c_str());
					//log::info("Playing pad2e: {}", a_this->pad2E); Always 0
					auto* EventName = a_this->GetFormEditorID();
					
					if (BlockAnimation(a_this, params)) {
						log::info("Returning nullptr");
						result = nullptr;
					}
				}

				//Actor* action_ref = params->actionRef->As<RE::Actor>();
    			//TESObjectREFR* target_ref = params->targetRef;

				return result;  
            }
        );
    }
}