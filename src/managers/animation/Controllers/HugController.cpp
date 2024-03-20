#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "events.hpp"
#include "spring.hpp"
#include "node.hpp"


namespace {

	const float MINIMUM_HUG_DISTANCE = 110.0;
	const float GRAB_ANGLE = 70;
	const float PI = 3.14159;

	bool DisallowHugs(Actor* actor) {
		bool jumping = IsJumping(actor);
		bool ragdolled = IsRagdolled(actor);
		bool busy = IsGtsBusy(actor);
		return jumping || ragdolled || busy;
	}
}

namespace Gts {
	HugAnimationController& HugAnimationController::GetSingleton() noexcept {
		static HugAnimationController instance;
		return instance;
	}

	std::string HugAnimationController::DebugName() {
		return "HugAnimationController";
	}

	std::vector<Actor*> HugAnimationController::GetHugTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (IsGtsBusy(pred)) {
			return {};
		}
		if (!pred) {
			return {};
		}
		auto charController = pred->GetCharController();
		if (!charController) {
			return {};
		}

		NiPoint3 predPos = pred->GetPosition();

		auto preys = find_actors();

		// Sort prey by distance
		sort(preys.begin(), preys.end(),
		     [predPos](const Actor* preyA, const Actor* preyB) -> bool
		{
			float distanceToA = (preyA->GetPosition() - predPos).Length();
			float distanceToB = (preyB->GetPosition() - predPos).Length();
			return distanceToA < distanceToB;
		});

		// Filter out invalid targets
		preys.erase(std::remove_if(preys.begin(), preys.end(),[pred, this](auto prey)
		{
			return !this->CanHug(pred, prey);
		}), preys.end());

		// Filter out actors not in front
		auto actorAngle = pred->data.angle.z;
		RE::NiPoint3 forwardVector{ 0.f, 1.f, 0.f };
		RE::NiPoint3 actorForward = RotateAngleAxis(forwardVector, -actorAngle, { 0.f, 0.f, 1.f });

		NiPoint3 predDir = actorForward;
		predDir = predDir / predDir.Length();
		preys.erase(std::remove_if(preys.begin(), preys.end(),[predPos, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - predPos;
			if (preyDir.Length() <= 1e-4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= 0; // 180 degress
		}), preys.end());

		// Filter out actors not in a truncated cone
		// \      x   /
		//  \  x     /
		//   \______/  <- Truncated cone
		//   | pred |  <- Based on width of pred
		//   |______|
		float predWidth = 70 * get_visual_scale(pred);
		float shiftAmount = fabs((predWidth / 2.0) / tan(GRAB_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(GRAB_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	bool HugAnimationController::CanHug(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		if (prey->IsDead()) {
			return false;
		}
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			return false;
		}
		if (IsCrawling(pred) || IsTransitioning(pred) || IsBeingHeld(prey)) {
			return false;
		}
		if (DisallowHugs(pred) || DisallowHugs(prey) ) {
			return false;
		}

		if (pred->AsActorState()->GetSitSleepState() == SIT_SLEEP_STATE::kIsSitting) { // disallow doing it when using furniture
			return false;	
		}

		float pred_scale = get_visual_scale(pred);
		// No need to check for BB scale in this case

		float sizedifference = GetSizeDifference(pred, prey, false, true);
		

		float MINIMUM_DISTANCE = MINIMUM_HUG_DISTANCE;
		float MINIMUM_HUG_SCALE = Action_Hug;

		if (pred->IsSneaking()) {
			MINIMUM_DISTANCE *= 1.5;
		}

		if (HasSMT(pred)) {
			MINIMUM_HUG_SCALE *= 0.80;
		}

		float balancemode = SizeManager::GetSingleton().BalancedMode();

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale)) {
			if (sizedifference > MINIMUM_HUG_SCALE) {
				if ((prey->formID != 0x14 && !CanPerformAnimationOn(pred, prey))) {
					return false;
				}
				if (!IsHuman(prey)) { // Allow hugs with humanoids only
					if (pred->formID == 0x14) {
						std::string_view message = std::format("You have no desire to hug {}", prey->GetDisplayFullName());
						TiredSound(pred, message); // Just no. We don't have Creature Anims.
					}
					return false;
				}
				return true;
			} else {
				if (pred->formID == 0x14) {
					std::string_view message = std::format("{} is too big to be hugged", prey->GetDisplayFullName());
					shake_camera(pred, 0.45, 0.30);
					TiredSound(pred, message);
				}
				return false;
			}
			return false;
		}
		return false;
	}

	void HugAnimationController::StartHug(Actor* pred, Actor* prey) {
		auto& hugging = HugAnimationController::GetSingleton();
		if (!hugging.CanHug(pred, prey)) {
			return;
		}
		static Timer HugTimer = Timer(12.0);
		if (IsActionOnCooldown(pred, CooldownSource::Action_Hugs)) {
			TiredSound(pred, "Hugs are on the cooldown");
			return;
		}

		UpdateFriendlyHugs(pred, prey, false);

		HugShrink::GetSingleton().HugActor(pred, prey);
		
		
		AnimationManager::StartAnim("Huggies_Try", pred);

		if (pred->IsSneaking() && !IsCrawling(pred)) {
			AnimationManager::StartAnim("Huggies_Try_Victim_S", prey); // GTSBEH_HugAbsorbStart_Sneak_V
		} else {
			AnimationManager::StartAnim("Huggies_Try_Victim", prey); //   GTSBEH_HugAbsorbStart_V
		}

		ApplyActionCooldown(pred, CooldownSource::Action_Hugs);
	}
}