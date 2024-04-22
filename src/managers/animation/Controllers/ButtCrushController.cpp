#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
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

	const float MINIMUM_BUTTCRUSH_DISTANCE = 95.0;
	const float BUTTCRUSH_ANGLE = 70;
	const float PI = 3.14159;

	void AttachToObjectBTask(Actor* giant, Actor* tiny) {
		SetBeingEaten(tiny, true);
		std::string name = std::format("ButtCrush_{}", tiny->formID);
		auto tinyhandle = tiny->CreateRefHandle();
		auto gianthandle = giant->CreateRefHandle();
		auto FrameA = Time::FramesElapsed();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 10.0) {
				return true;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			auto node = find_node(giantref, "AnimObjectB");
			if (!node) {
				return false;
			}

			ForceRagdoll(tinyref, false);
			float stamina = GetAV(giantref, ActorValue::kStamina);
			DamageAV(giantref, ActorValue::kStamina, 0.04 * GetButtCrushCost(giant));

			ApplyActionCooldown(giantref, CooldownSource::Action_ButtCrush); // Set butt crush on the cooldown

			if (stamina <= 2.0 && !IsChangingSize(giantref)) {
				AnimationManager::StartAnim("ButtCrush_Attack", giantref); // Try to Abort it
			}

			if (GetAV(giantref, ActorValue::kHealth) <= 1.0 || giantref->IsDead()) {
				SetButtCrushSize(giantref, 0.0, true);
				PushActorAway(giantref, giantref, 1.0);
				PushActorAway(tinyref, tinyref, 1.0);

				SetBeingEaten(tinyref, false);
				EnableCollisions(tiny);

				SpawnCustomParticle(giantref, ParticleType::Red, NiPoint3(), "NPC Root [Root]", 3.0);
				SpawnParticle(giantref, 4.60, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), giantref->GetPosition(), get_visual_scale(giantref) * 4.0, 7, nullptr);
				Runtime::PlaySoundAtNode_FallOff("TinyCalamity_Impact", giantref, 1.0, 1.0, "NPC COM [COM ]", 0.10 * get_visual_scale(giantref));
				Rumbling::Once("ButtCrushDeath", giantref, 128.0, 0.25, "NPC Root [Root]");
				return false;
			}

			auto coords = node->world.translate;
			if (!IsCrawling(giantref)) {
				float HH = HighHeelManager::GetHHOffset(giantref).Length();
				coords.z -= HH;
			}
			if (!IsButtCrushing(giantref)) {
				SetBeingEaten(tiny, false);
				EnableCollisions(tiny);
				return false;
			}
			if (!AttachTo_NoForceRagdoll(giantref, tinyref, coords)) {
				SetBeingEaten(tiny, false);
				EnableCollisions(tiny);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingEaten(tiny, false);
				EnableCollisions(tiny);
				return false;
			}
			return true;
		});
	}
}

namespace Gts {
	ButtCrushController& ButtCrushController::GetSingleton() noexcept {
		static ButtCrushController instance;
		return instance;
	}

	std::string ButtCrushController::DebugName() {
		return "ButtCrushController";
	}

	void ButtCrushController::ButtCrush_OnCooldownMessage(Actor* giant) {
		float cooldown = GetRemainingCooldown(giant, CooldownSource::Action_ButtCrush);
		if (giant->formID == 0x14) {
			if (!IsCrawling(giant) && !giant->IsSneaking()) {
				std::string message = std::format("Butt Crush is on a cooldown: {:.1f} sec", cooldown);
				TiredSound(giant, message);
			} else if (giant->IsSneaking()) {
				std::string message = std::format("Knee Crush is on a cooldown: {:.1f} sec", cooldown);
				TiredSound(giant, message);
			} else {
				std::string message = std::format("Breast Crush is on a cooldown: {:.1f} sec", cooldown);
				TiredSound(giant, message);
			}
		}
	}

	std::vector<Actor*> ButtCrushController::GetButtCrushTargets(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (!CanPerformAnimation(pred, 2)) {
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
			return !this->CanButtCrush(pred, prey);
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
		float shiftAmount = fabs((predWidth / 2.0) / tan(BUTTCRUSH_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(BUTTCRUSH_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	bool ButtCrushController::CanButtCrush(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}

		if (prey->IsDead()) {
			return false;
		}
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			return false;
		}

		float pred_scale = get_visual_scale(pred);
		float sizedifference = GetSizeDifference(pred, prey, SizeType::VisualScale, true, false);

		float MINIMUM_BUTTCRUSH_SCALE = Action_Booty;
		float MINIMUM_DISTANCE = MINIMUM_BUTTCRUSH_DISTANCE;
		if (IsCrawling(pred)) {
			MINIMUM_BUTTCRUSH_SCALE *= 1.25;
		}

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= MINIMUM_DISTANCE * pred_scale && sizedifference < MINIMUM_BUTTCRUSH_SCALE) {
			std::string_view message = std::format("{} is too big for Butt Crush", prey->GetDisplayFullName());
			if (IsCrawling(pred)) {
				message = std::format("{} is too big for Breast Crush", prey->GetDisplayFullName());
			}
			TiredSound(pred, message);
			return false;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && sizedifference >= MINIMUM_BUTTCRUSH_SCALE) {
			if (IsFlying(prey)) {
				return false; // Disallow to butt crush flying dragons
			}
			if ((prey->formID != 0x14 && !CanPerformAnimationOn(pred, prey))) {
				std::string_view message = std::format("{} is Essential", prey->GetDisplayFullName());
				TiredSound(pred, message);
				return false;
			}
			return true;
		} else {
			return false;
		}
	}

	void ButtCrushController::StartButtCrush(Actor* pred, Actor* prey) {
		auto& buttcrush = ButtCrushController::GetSingleton();
		
		if (!buttcrush.CanButtCrush(pred, prey)) {
			return;
		}

		if (CanDoButtCrush(pred, false) && !IsBeingHeld(pred, prey)) {
			prey->NotifyAnimationGraph("GTS_EnterFear");
			
			if (GetSizeDifference(pred, prey, SizeType::VisualScale, false, false) < Action_Booty) {
				ShrinkUntil(pred, prey, 3.4, 0.25, true);
				return;
			}

			DisableCollisions(prey, pred);

			float WasteStamina = 60.0 * GetButtCrushCost(pred);
			DamageAV(pred, ActorValue::kStamina, WasteStamina);

			AttachToObjectBTask(pred, prey);

			ApplyActionCooldown(pred, CooldownSource::Action_ButtCrush); // Set butt crush on the cooldown
			AnimationManager::StartAnim("ButtCrush_Start", pred);
		} else {
			ButtCrush_OnCooldownMessage(pred);
		}
	}
}