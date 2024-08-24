#include "managers/animation/Controllers/ThighCrushController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/ai_SelectAction.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
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

	const float MINIMUM_THIGH_DISTANCE = 58.0;
	const float THIGH_ANGLE = 75;
	const float PI = 3.14159;
}

namespace Gts {
	ThighCrushController& ThighCrushController::GetSingleton() noexcept {
		static ThighCrushController instance;
		return instance;
	}

	std::string ThighCrushController::DebugName() {
		return "ThighCrushController";
	}

	std::vector<Actor*> ThighCrushController::GetThighTargetsInFront(Actor* pred, std::size_t numberOfPrey, bool ai_triggered) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
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
		preys.erase(std::remove_if(preys.begin(), preys.end(),[pred, this, ai_triggered](auto prey)
		{
			return !this->CanThighCrush(pred, prey, ai_triggered);
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
		float shiftAmount = fabs((predWidth / 2.0) / tan(THIGH_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(THIGH_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	bool ThighCrushController::CanThighCrush(Actor* pred, Actor* prey, bool ai_triggered) {
		if (pred == prey) {
			return false;
		}

		if (prey->IsDead()) {
			return false;
		}
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			return false;
		}
		if (IsCrawling(pred) || IsTransitioning(pred) || IsBeingHeld(pred, prey)) {
			return false;
		}

		if (pred->AsActorState()->GetSitSleepState() == SIT_SLEEP_STATE::kIsSitting) { // disallow doing it when using furniture
			return false;	
		}

		float pred_scale = get_visual_scale(pred);
		// No need to check for BB scale in this case

		float sizedifference = GetSizeDifference(pred, prey, SizeType::VisualScale, false, true);
		
		float MINIMUM_DISTANCE = MINIMUM_THIGH_DISTANCE + HighHeelManager::GetBaseHHOffset(pred).Length();
		float MINIMUM_CRUSH_SCALE = Action_ThighCrush;

		if (ai_triggered) {
			MINIMUM_CRUSH_SCALE = 0.92;
		}

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale)) {
			if (sizedifference > MINIMUM_CRUSH_SCALE) {
				if ((prey->formID != 0x14 && !CanPerformAnimationOn(pred, prey, false))) {
					return false;
				}
				return true;
			} else {
				return false;
			}
			return false;
		}
		return false;
	}

	void ThighCrushController::StartThighCrush(Actor* pred, Actor* prey, bool ai_triggered) {
		auto& ThighCrush = ThighCrushController::GetSingleton();
		if (!ThighCrush.CanThighCrush(pred, prey, ai_triggered)) {
			return;
		}
		if (IsThighCrushing(pred)) {
			return;
		}

		AnimationManager::StartAnim("ThighLoopEnter", pred);
		AI_StartThighCrushTask(pred);

		log::info("Starting Thigh Crush between {} and {}", pred->GetDisplayFullName(), prey->GetDisplayFullName());
	}
}