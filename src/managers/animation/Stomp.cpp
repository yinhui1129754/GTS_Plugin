// Animation: Stomp
//  - Stages
//    - "GTSstompimpactR",          // [0] stomp impacts, strongest effect
//    - "GTSstompimpactL",          // [1]
//    - "GTSstomplandR",            // [2] when landing after stomping, decreased power
//    - "GTSstomplandL",            // [3]
//    - "GTSstompstartR",           // [4] For starting loop of camera shake and air rumble sounds
//    - "GTSstompstartL",           // [5]
//    - "GTSStompendR",             // [6] disable loop of camera shake and air rumble sounds
//    - "GTSStompendL",             // [7]
//    - "GTS_Next",                 // [8]
//    - "GTSBEH_Exit",              // [9] Another disable

#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Stomp.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/height.hpp"
#include "scale/scale.hpp"

#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";
	std::random_device rd;
	std::mt19937 e2(rd());

	std::vector<Actor*> FindSquished(Actor* giant) {
		/*
		Find actor that are being pressed underfoot
		*/
		std::vector<Actor*> result = {};
		if (!giant) {
			return result;
		}
		float giantScale = get_visual_scale(giant);
		auto giantLoc = giant->GetPosition();
		for (auto tiny: find_actors()) {
			if (tiny) {
				float tinyScale = get_visual_scale(tiny);
				float scaleRatio = giantScale / tinyScale;
				
				float actorRadius = 35.0;
				auto bounds = get_bound(tiny);
				if (bounds) {
					actorRadius = (bounds->extents.x + bounds->extents.y + bounds->extents.z) / 6.0;
				}

				actorRadius *= tinyScale;
				
				if (scaleRatio > 3.5) {
					// 3.5 times bigger
					auto tinyLoc = tiny->GetPosition();
					auto distance = (giantLoc - tinyLoc).Length() - actorRadius;
					if (distance < giantScale * 15.0) {
						// About 1.5 the foot size
						result.push_back(tiny);
					}
				}
			}
		}
		return result;
	}

	void MoveUnderFoot(Actor* giant, std::string_view node) {
		auto footNode = find_node(giant, RNode);
		if (footNode) {
			auto footPos = footNode->world.translate;
			for (auto tiny: FindSquished(giant)) {
				std::uniform_real_distribution<> dist(-10, 10);
				float dx = dist(e2);
				float dy = dist(e2);
				auto randomOffset = NiPoint3(dx, dy, 0.0);
				tiny->SetPosition(footPos + randomOffset, true);
			}
		}
	}

	/*
	Will keep the tiny in place for a second
	*/
	void KeepInPlace(Actor* giant) {
		for (auto tiny: FindSquished(giant)) {
			auto giantRef = giant->CreateRefHandle();
			auto tinyRef = tiny->CreateRefHandle();
			auto currentPos = tiny->GetPosition();
			TaskManager::RunFor(1.5, [=](const auto& data){
				if (!tinyRef) {
					return false;	
				}
				if (!giantRef) {
					return false;	
				}
				AttachTo(giantRef, tinyRef, currentPos);
				return true;
			});
		}
	}

	void Stomp_DoEverything(Actor* giant, bool right, float animSpeed, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float shake = 1.0;
		float dust = 1.25;
		if (HasSMT(giant)) {
			shake = 4.0;
			dust = 1.45;
		}

		GRumble::Once("StompR", giant, 2.20 * shake, 0.0, Node);

		// TO ANDY: i commented it out for tests
		//MoveUnderFoot(giant, Node); 

		DoDamageEffect(giant, Damage_Stomp * perk, Radius_Stomp, 10, 0.25, Event, 1.0, Source);
		DoDustExplosion(giant, dust + (animSpeed * 0.05), Event, Node);
		DoFootstepSound(giant, 1.0 + animSpeed/8, Event, Node);
		
		DrainStamina(giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.8); // cancel stamina drain

		DoLaunch(giant, 0.80 * perk, 1.35 * animSpeed, Event);

		if (right) {
			FootGrindCheck_Right(giant, Radius_Stomp, false);
		} else {
			FootGrindCheck_Left(giant, Radius_Stomp, false);
		}
	}

	void Stomp_Land_DoEverything(Actor* giant, float animSpeed, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float shake = 1.0;
		float dust = 0.85;
		
		if (HasSMT(giant)) {
			dust = 1.35;
			shake = 4.0;
		}
		
		GRumble::Once(rumble, giant, 1.25 * shake, 0.05, Node);
		DoDamageEffect(giant, Damage_Stomp * perk, Radius_Stomp, 25, 0.25, Event, 1.0, DamageSource::CrushedRight);
		DoDustExplosion(giant, dust + (animSpeed * 0.05), Event, Node);
		DoFootstepSound(giant, 1.0 + animSpeed/14, Event, RNode);
		
		DoLaunch(giant, 0.75 * perk, 1.5 + animSpeed/4, Event);
		KeepInPlace(giant);
		
	}

///////////////////////////////////////////////////////////////////////////////////////////////////// Events

	void GTSstompstartR(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.8);
		GRumble::Start("StompR", &data.giant, 0.35, 0.15, RNode);
		ManageCamera(&data.giant, true, CameraTracking::R_Foot);

	}

	void GTSstompstartL(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.8);
		GRumble::Start("StompL", &data.giant, 0.45, 0.15, LNode);
		ManageCamera(&data.giant, true, CameraTracking::L_Foot);
	}

	void GTSstompimpactR(AnimationEventData& data) {
		Stomp_DoEverything(&data.giant, true, data.animSpeed, FootEvent::Right, DamageSource::CrushedRight, RNode, "StompR");
	}

	void GTSstompimpactL(AnimationEventData& data) {
		Stomp_DoEverything(&data.giant, false, data.animSpeed, FootEvent::Left, DamageSource::CrushedLeft, LNode, "StompL");
	}

	void GTSstomplandR(AnimationEventData& data) {
		Stomp_Land_DoEverything(&data.giant, data.animSpeed, FootEvent::Right, DamageSource::CrushedRight, RNode, "StompR_L");
	}

	void GTSstomplandL(AnimationEventData& data) {
		Stomp_Land_DoEverything(&data.giant, data.animSpeed, FootEvent::Right, DamageSource::CrushedRight, RNode, "StompL_L");
	}

	void GTSStompendR(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		//BlockFirstPerson(&data.giant, false);
	}

	void GTSStompendL(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		//BlockFirstPerson(&data.giant, false);
	}

	void GTS_Next(AnimationEventData& data) {
		GRumble::Stop("StompR", &data.giant);
		GRumble::Stop("StompL", &data.giant);
		GRumble::Stop("StompRL", &data.giant);
		GRumble::Stop("StompLL", &data.giant);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		GRumble::Stop("StompR", &data.giant);
		GRumble::Stop("StompL", &data.giant);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.8);
		DrainStamina(&data.giant, "StaminaDrain_StrongStomp", "DestructionBasics", false, 2.8);
		ManageCamera(&data.giant, false, CameraTracking::L_Foot);
		ManageCamera(&data.giant, false, CameraTracking::R_Foot);
	}

	void RightStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		float WasteStamina = 25.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("StompRight", player);
		} else {
			TiredSound(player, "You're too tired to perform stomp");
		}
	}

	void LeftStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		float WasteStamina = 25.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("StompLeft", player);
		} else {
			TiredSound(player, "You're too tired to perform stomp");
		}
	}
}

namespace Gts
{
	void AnimationStomp::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSstompimpactR", "Stomp", GTSstompimpactR);
		AnimationManager::RegisterEvent("GTSstompimpactL", "Stomp", GTSstompimpactL);
		AnimationManager::RegisterEvent("GTSstomplandR", "Stomp", GTSstomplandR);
		AnimationManager::RegisterEvent("GTSstomplandL", "Stomp", GTSstomplandL);
		AnimationManager::RegisterEvent("GTSstompstartR", "Stomp", GTSstompstartR);
		AnimationManager::RegisterEvent("GTSstompstartL", "Stomp", GTSstompstartL);
		AnimationManager::RegisterEvent("GTSStompendR", "Stomp", GTSStompendR);
		AnimationManager::RegisterEvent("GTSStompendL", "Stomp", GTSStompendL);
		AnimationManager::RegisterEvent("GTS_Next", "Stomp", GTS_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "Stomp", GTSBEH_Exit);

		InputManager::RegisterInputEvent("RightStomp", RightStompEvent);
		InputManager::RegisterInputEvent("LeftStomp", LeftStompEvent);
	}

	void AnimationStomp::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StompRight", "Stomp", "GtsModStompAnimRight");
		AnimationManager::RegisterTrigger("StompLeft", "Stomp", "GtsModStompAnimLeft");
	}
}