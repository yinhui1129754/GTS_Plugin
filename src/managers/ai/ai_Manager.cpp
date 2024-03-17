#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/ai_SelectAction.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/ai_Manager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"

namespace {
	const float MINIMUM_STOMP_DISTANCE = 50.0;
	const float MINIMUM_STOMP_SCALE_RATIO = 1.75;
	const float STOMP_ANGLE = 50;
	const float PI = 3.14159;

	bool ProtectFollowers(Actor* giant, Actor* tiny) {
		bool NPC = Persistent::GetSingleton().FollowerProtection;
		if (tiny->formID != 0x14 && NPC && (IsTeammate(giant)) && (IsTeammate(tiny))) {
			return true; // Disallow NPC's to perform stomps on followers
		}
		return false;
	}

	void AI_PerformRandomVore(Actor* pred) {
		if (IsGtsBusy(pred)) {
			return; // No Vore attempts if in GTS_Busy
		}
		log::info("PerformRandomVore started");
		auto& VoreManager = Vore::GetSingleton();

		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(pred, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(pred)/3);
		}
		for (auto actor: find_actors()) {
			if (!actor->Is3DLoaded() || actor->IsDead()) {
				return;
			}
			int Requirement = 8 * SizeManager::GetSingleton().BalancedMode();

			int random = rand() % Requirement;
			int trigger_threshold = 2;
			if (random <= trigger_threshold) {
				log::info("Random < threshold");
				std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
				for (auto prey: preys) {
					VoreManager.StartVore(pred, prey);
					log::info("StartVore called, can vore: {}", VoreManager.CanVore(pred, prey));
				}
			}
		}
	}	

	void AI_ChanceToStartVore() {
		if (!Persistent::GetSingleton().Vore_Ai) {
			return;
		}
		std::vector<Actor*> AbleToVore = {};
		auto& persist = Persistent::GetSingleton();
		
		for (auto actor: find_actors()) {
			if (actor->formID != 0x14 && IsTeammate(actor) && (actor->IsInCombat() || !persist.vore_combatonly) || (EffectsForEveryone(actor) && IsFemale(actor))) {
				AbleToVore.push_back(actor);
			}
		}
		if (!AbleToVore.empty()) {
			int idx = rand() % AbleToVore.size();
			Actor* voreActor = AbleToVore[idx];
			if (voreActor) {
				AI_PerformRandomVore(voreActor);
				log::info("Found actor for vore");
			}
		}
	}

}


namespace Gts {
	AiData::AiData(Actor* giant) : giant(giant? giant->CreateRefHandle() : ActorHandle()) {
	}

	AiManager& AiManager::GetSingleton() noexcept {
		static AiManager instance;
		return instance;
	}

	std::string AiManager::DebugName() {
		return "AiManager";
	}

	void AiManager::Update() {
		auto profiler = Profilers::Profile("Ai: Update");
		static Timer ActionTimer = Timer(0.80);
		static Timer VoreTimer = Timer(2.50); // Random Vore once per 2.5 sec
		if (VoreTimer.ShouldRunFrame()) {
			AI_ChanceToStartVore();
		}
		if (ActionTimer.ShouldRun()) {
			auto& persist = Persistent::GetSingleton();
			for (auto actor: find_actors()) {
				std::vector<Actor*> AbleToAct = {};
				for (auto actor: find_actors()) {
					if (IsTeammate(actor) && actor->formID != 0x14 && IsFemale(actor) || (EffectsForEveryone(actor) && IsFemale(actor))) {
						if (actor->IsInCombat() || !persist.vore_combatonly) {
							AbleToAct.push_back(actor);
						}
					}
				}
				if (!AbleToAct.empty()) {
					int idx = rand() % AbleToAct.size();
					Actor* Performer = AbleToAct[idx];
					if (Performer) {
						AI_TryAction(Performer);
					}
				}
			}
		}
	}

	std::vector<Actor*> AiManager::RandomStomp(Actor* pred, std::size_t numberOfPrey) {
		// Get targets in front
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
			return !this->CanStomp(pred, prey);
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
		float shiftAmount = fabs((predWidth / 2.0) / tan(STOMP_ANGLE/2.0));
		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(STOMP_ANGLE*PI/180.0);
		}), preys.end());
		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}
		return preys;
	}

	bool AiManager::CanStomp(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		if (ProtectFollowers(pred, prey)) {
			return false;
		}
		if (IsGtsBusy(pred)) {
			return false;
		}
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore || !CanPerformAnimationOn(pred, prey)) {
			return false;
		}
		float pred_scale = get_visual_scale(pred) * GetSizeFromBoundingBox(pred);
		float prey_scale = get_visual_scale(prey) * GetSizeFromBoundingBox(prey);

		float bonus = 1.0;
		if (IsCrawling(pred)) {
			bonus = 2.0; // +100% stomp distance
		}
		if (prey->IsDead() && pred_scale/prey_scale < 8.0) {
			return false;
		}

		float sizedifference = pred_scale/prey_scale;

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale * bonus) && pred_scale/prey_scale < MINIMUM_STOMP_SCALE_RATIO) {
			return false;
		}
		if (prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale * bonus)
		    && pred_scale/prey_scale > MINIMUM_STOMP_SCALE_RATIO
		    && prey_distance > 25.0) { // We don't want the Stomp to be too close
			return true;
		} else {
			return false;
		}
	}

	void AiManager::Reset() {
		this->data_ai.clear();
	}

	void AiManager::ResetActor(Actor* actor) {
		this->data_ai.erase(actor->formID);
	}

	AiData& AiManager::GetAiData(Actor* giant) {
		// Create it now if not there yet
		this->data_ai.try_emplace(giant->formID, giant);

		return this->data_ai.at(giant->formID);
	}
}
