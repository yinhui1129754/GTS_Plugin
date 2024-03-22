#include "hooks/PreventAnimations.hpp"
#include "data/transient.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"
#include "utils/debug.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    const float KillMove_Threshold_High = 2.00f; // If GTS/Tiny size ration is > than 2 times = disallow killmove 
    const float KillMove_Threshold_Low = 0.75f; // If Tiny/GTS size ratio is < than 0.75 = disallow killmove on GTS

	// Actions that we want to prevent
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
			if (victim) {
				Actor* victimref = skyrim_cast<Actor*>(victim);
				if (victimref) {
					float size_difference = GetSizeDifference(performer, victimref, SizeType::GiantessScale, true, false);
					if (size_difference > KillMove_Threshold_High || size_difference < KillMove_Threshold_Low) {
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

		if (performer) {
			if (PreventKillMove(Form, params, performer, params->targetRef)) {
				log::info("KILLMOVE PREVENTED");
				return true;
			}

			if (performer->formID == 0x14 || !IsGtsBusy(performer)) {
                // Do not affect the player: we already disable player controls through other hook
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
        static FunctionHook<TESIdleForm*(TESIdleForm* a_this, ConditionCheckParams* params, void* unk3)>IdleFormHook (        
			REL::RelocationID(24067, 24570),
			// 24067 = sub_140358150 (SE)
			// 24570 = FUN_14036ec80 (AE)

			// OLD HOOKS BELOW (used CallHook on these, but for some reason it's not called on AE, so we hook function itself)

			//REL::RelocationID(24068, 24571), REL::Relocate(0x5E, 0x53),
			// 24068 = 0x140358250 [SE] ; 0x140358250 - 0x1403582ae = 0x5E
			// 24571 = 0x14036ee00 [AE] ; 0x14036ee00 - 0x14036ee53 = 0x53
			[](TESIdleForm* a_this, ConditionCheckParams* params, void* unk3) {
                // Return nullptr When we don't want specific anims to happen
                // This hook prevents them from playing (KillMoves and Sheathe/Unsheathe/Jump anims)
				
				auto* result = IdleFormHook(a_this, params, unk3);

				if (a_this) {
					auto* EventName = a_this->GetFormEditorID();
					
					if (BlockAnimation(a_this, params)) {
                        log::info("Performer: {}", params->actionRef->GetDisplayFullName());
						log::info("Returning nullptr");
						result = nullptr;
					}
				}

				return result;  
            }
        );
    }
}