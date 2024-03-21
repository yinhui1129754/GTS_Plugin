#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/animation/Grab.hpp"
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

	const float MINIMUM_GRAB_DISTANCE = 85.0;
	const float GRAB_ANGLE = 70;
	const float PI = 3.14159;
}

namespace Gts {
	GrabAnimationController& GrabAnimationController::GetSingleton() noexcept {
		static GrabAnimationController instance;
		return instance;
	}

	std::string GrabAnimationController::DebugName() {
		return "GrabAnimationController";
	}

	std::vector<Actor*> GrabAnimationController::GetGrabTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
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
			return !this->CanGrab(pred, prey);
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

	bool GrabAnimationController::CanGrab(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		if (prey->IsDead()) {
			return false;
		}
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			return false;
		}
		if (IsGtsBusy(pred)) {
			return false;
		}

		float pred_scale = get_visual_scale(pred);

		float sizedifference = GetSizeDifference(pred, prey, true, false);

		float MINIMUM_GRAB_SCALE = Action_Grab;
		float MINIMUM_DISTANCE = MINIMUM_GRAB_DISTANCE;

		if (pred->formID == 0x14 && IsTeammate(prey) && !GetControlledActor()) {
			float sizedifference_reverse = GetSizeDifference(prey, pred, true, false);
			log:::info("SD check passed");
			if (sizedifference_reverse >= Action_Grab) {
				ControlAnother(prey, false);
				log:::info("Controlling {}", prey->GetDisplayFullName());
				prey = pred;
				log:::info("New Prey {}", prey->GetDisplayFullName());
				pred = GetControlledActor();
				log:::info("New Pred {}", pred->GetDisplayFullName());
				// Switch roles
			}
		}

		if (HasSMT(pred)) {
			MINIMUM_DISTANCE *= 1.75;
		}

		float balancemode = SizeManager::GetSingleton().BalancedMode();

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= MINIMUM_DISTANCE * pred_scale && sizedifference < MINIMUM_GRAB_SCALE) {
			Notify("{} is too big to be grabbed.", prey->GetDisplayFullName());
			return false;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && sizedifference > MINIMUM_GRAB_SCALE) {
			if ((prey->formID != 0x14 && !CanPerformAnimationOn(pred, prey))) {
				return false;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}

	void GrabAnimationController::StartGrab(Actor* pred, Actor* prey) {
		auto& grabbing = GrabAnimationController::GetSingleton();
		if (!grabbing.CanGrab(pred, prey)) {
			return;
		}

		if (GetControlledActor()) {
			log:::info("Controlled Actor", GetControlledActor());
			prey = pred;
			log:::info("Start New Prey: {}", prey->GetDisplayFullName());
			pred = GetControlledActor();
			log:::info("Start New Pred: {}", pred->GetDisplayFullName());
		}

		StaggerActor(pred, prey, 100.0f);

		float shrinkrate = 0.18;

		if (pred->IsSneaking()) {
			shrinkrate = 0.13;
		}

		if (GetSizeDifference(pred, prey, false, false) < Action_Grab) {
			ShrinkUntil(pred, prey, 12.0, shrinkrate, true);
			return;
		}

		Grab::GetSingleton().GrabActor(pred, prey);
		
		Utils_UpdateHighHeelBlend(pred, false);
		AnimationManager::StartAnim("GrabSomeone", pred);
	}
}