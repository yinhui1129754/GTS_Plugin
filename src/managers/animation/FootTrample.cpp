#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/FootTrample.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/audio/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void DeplenishStamina(Actor* giant, float WasteStamina) {
		DamageAV(giant, ActorValue::kStamina, WasteStamina * GetWasteMult(giant));
	}

	void DoSounds(Actor* giant, float animspeed, std::string_view feet) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = 8.0;
		}
		float scale = get_visual_scale(giant);
		Runtime::PlaySoundAtNode("HeavyStompSound", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlFootstep", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlRumble", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
	}

	void FootTrample_Stage1(Actor* giant, bool right, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float smt = 1.0;
		float dust = 1.0;
		
		if (HasSMT(giant)) {
			dust = 1.25;
			smt = 1.5;
		}

		float Start = Time::WorldTimeElapsed();
		ActorHandle giantHandle = giant->CreateRefHandle();
		std::string taskname = std::format("TrampleAttack_{}", giant->formID);

		TaskManager::RunFor(taskname, 1.0, [=](auto& update){ // Needed because anim has a bit wrong timing(s)
			if (!giantHandle) {
				return false;
			}

			float Finish = Time::WorldTimeElapsed();
			auto giant = giantHandle.get().get();

			if (Finish - Start > 0.06) { 
				DoDamageEffect(giant, Damage_Trample * perk, Radius_Trample, 100, 0.10, Event, 1.10, Source);
				DrainStamina(giant, "StaminaDrain_Trample", "DestructionBasics", true, 0.6); // start stamina drain

				float shake_power = Rumble_Trample_Stage1 * smt * GetHighHeelsBonusDamage(giant, true);
				
				Rumbling::Once(rumble, giant, shake_power, 0.0, Node, 0.0);
				LaunchTask(giant, 0.65 * perk, 1.15 * perk, Event);
				DoDustExplosion(giant, dust * smt, Event, Node);
				DoFootstepSound(giant, 1.0, Event, Node);
				
				FootGrindCheck(giant, Radius_Trample, true, right);

				return false;
			}
			return true;
		});
	}

	void FootTrample_Stage2(Actor* giant, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float dust = 1.15;
		float smt = 1.0;
		
		if (HasSMT(giant)) {
			smt = 1.5;
			dust *= 1.25;
		}

		float shake_power = Rumble_Trample_Stage2 * smt * GetHighHeelsBonusDamage(giant, true);

		Rumbling::Once(rumble, giant, shake_power, 0.0, Node, 1.1);
		DoDamageEffect(giant, Damage_Trample_Repeat * perk, Radius_Trample_Repeat, 1, 0.12, Event, 1.10, Source);
		DoFootstepSound(giant, 1.0, Event, Node);
		DoDustExplosion(giant, dust * smt, Event, Node);
		DoLaunch(giant, 0.85 * perk, 1.85 * perk, Event);
		DeplenishStamina(giant, 30.0);
	}

	void FootTrample_Stage3(Actor* giant, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float dust = 1.65;
		float smt = 1.0;
		
		if (HasSMT(giant)) {
			smt = 1.5;
			dust *= 1.25;
		}

		float shake_power = Rumble_Trample_Stage3 * smt * GetHighHeelsBonusDamage(giant, true);

		Rumbling::Once(rumble, giant, shake_power, 0.0, Node, 1.2);
		DoDamageEffect(giant, Damage_Trample_Finisher * perk, Radius_Trample_Finisher, 1, 0.25, Event, 0.85, Source);
		DoLaunch(giant, 1.25 * perk, 4.20 * perk, Event);
		DoFootstepSound(giant, 1.15, Event, Node);
		DoDustExplosion(giant, dust * smt, Event, Node);

		DeplenishStamina(giant, 100.0);

		DoSounds(giant, 1.25, Node);
	}

	/////////////////////////////////////////////////////////
	// EVENTS
	////////////////////////////////////////////////////////

	void GTS_Trample_Leg_Raise_L(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.0) {
			data.animSpeed = 1.3;
		}
	}
	void GTS_Trample_Leg_Raise_R(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.0) {
			data.animSpeed = 1.3;
		}
	}

	void GTS_Trample_Cam_Start_L(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::L_Foot);
	}
	void GTS_Trample_Cam_Start_R(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::R_Foot);
	}

	void GTS_Trample_Cam_End_L(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::L_Foot);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 0.6);

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}
	void GTS_Trample_Cam_End_R(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::R_Foot);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 0.6);

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}

////////////////////////////////////////////////////////////D A M A G E

	void GTS_Trample_Footstep_L(AnimationEventData& data) { // Stage 1 footsteps
		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;

		FootTrample_Stage1(&data.giant, false, FootEvent::Left, DamageSource::CrushedLeft, LNode, "TrampleL");
	}
	void GTS_Trample_Footstep_R(AnimationEventData& data) { // stage 1 footsteps
		FootTrample_Stage1(&data.giant, true, FootEvent::Right, DamageSource::CrushedRight, RNode, "TrampleR");

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}

	void GTS_Trample_Impact_L(AnimationEventData& data) { // Stage 2 repeating footsteps
		FootTrample_Stage2(&data.giant, FootEvent::Left, DamageSource::CrushedLeft, LNode, "Trample2_L");

		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.0) {
			data.animSpeed = 1.15;
		}
	}

	void GTS_Trample_Impact_R(AnimationEventData& data) { // Stage 2 repeating footsteps
		FootTrample_Stage2(&data.giant, FootEvent::Right, DamageSource::CrushedRight, RNode, "Trample2_R");

		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.00) {
			data.animSpeed = 1.15;
		}
	}

	void GTS_Trample_Finisher_L(AnimationEventData& data) { // last hit that deals huge chunk of damage
		//Rumbling::Stop("Trample2_L", &data.giant);
		FootTrample_Stage3(&data.giant, FootEvent::Left, DamageSource::CrushedLeft, LNode, "Trample2_L");
	}
	void GTS_Trample_Finisher_R(AnimationEventData& data) { // last hit that deals huge chunk of damage
		//Rumbling::Stop("Trample2_R", &data.giant);
		FootTrample_Stage3(&data.giant, FootEvent::Right, DamageSource::CrushedRight, RNode, "Trample2_R");
	}

	/////////////////////////////////////////////////////////// Triggers

	void TrampleLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (IsCrawling(player) || player->IsSneaking() || IsProning(player)) {
			return;
		}
		float WasteStamina = 35.0 * GetWasteMult(player);

		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("TrampleL", player);
		} else {
			TiredSound(player, "You're too tired to perform trample");
		}
	}

	void TrampleRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		if (IsCrawling(player) || player->IsSneaking() || IsProning(player)) {
			return;
		}
		float WasteStamina = 35.0 * GetWasteMult(player);
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("TrampleR", player);
		} else {
			TiredSound(player, "You're too tired to perform trample");
		}
	}
}

namespace Gts
{
	void AnimationFootTrample::RegisterEvents() {
		InputManager::RegisterInputEvent("TrampleLeft", TrampleLeftEvent);
		InputManager::RegisterInputEvent("TrampleRight", TrampleRightEvent);

		AnimationManager::RegisterEvent("GTS_Trample_Leg_Raise_L", "Trample", GTS_Trample_Leg_Raise_L);
		AnimationManager::RegisterEvent("GTS_Trample_Leg_Raise_R", "Trample", GTS_Trample_Leg_Raise_R);

		AnimationManager::RegisterEvent("GTS_Trample_Cam_Start_L", "Trample", GTS_Trample_Cam_Start_L);
		AnimationManager::RegisterEvent("GTS_Trample_Cam_Start_R", "Trample", GTS_Trample_Cam_Start_R);

		AnimationManager::RegisterEvent("GTS_Trample_Cam_End_L", "Trample", GTS_Trample_Cam_End_L);
		AnimationManager::RegisterEvent("GTS_Trample_Cam_End_R", "Trample", GTS_Trample_Cam_End_R);

		AnimationManager::RegisterEvent("GTS_Trample_Impact_L", "Trample", GTS_Trample_Impact_L);
		AnimationManager::RegisterEvent("GTS_Trample_Impact_R", "Trample", GTS_Trample_Impact_R);

		AnimationManager::RegisterEvent("GTS_Trample_Footstep_L", "Trample", GTS_Trample_Footstep_L);
		AnimationManager::RegisterEvent("GTS_Trample_Footstep_R", "Trample", GTS_Trample_Footstep_R);

		AnimationManager::RegisterEvent("GTS_Trample_Finisher_L", "Trample", GTS_Trample_Finisher_L);
		AnimationManager::RegisterEvent("GTS_Trample_Finisher_R", "Trample", GTS_Trample_Finisher_R);
	}

	void AnimationFootTrample::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TrampleL", "Trample", "GTSBeh_Trample_L");
		AnimationManager::RegisterTrigger("TrampleR", "Trample", "GTSBeh_Trample_R");

		AnimationManager::RegisterTrigger("TrampleStartL", "Trample", "GTSBEH_Trample_Start_L");
		AnimationManager::RegisterTrigger("TrampleStartR", "Trample", "GTSBEH_Trample_Start_R");
	}
}