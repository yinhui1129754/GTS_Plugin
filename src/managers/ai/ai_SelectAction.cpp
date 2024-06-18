#include "managers/animation/Controllers/ThighCrushController.hpp"
#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/ai_PerformAction.hpp"
#include "managers/ai/ai_SelectAction.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/ai_Manager.hpp"
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
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"

namespace {
	bool AI_CanHugCrush(Actor* giant, Actor* tiny, int rng) {
		int crush_rng = rand() % 4;

		float health = GetHealthPercentage(tiny);
		float HpThreshold = GetHugCrushThreshold(giant, tiny);

		bool low_hp = (health <= HpThreshold);
		bool allow_perform = (tiny->formID != 0x14 && IsHostile(giant, tiny)) || (rng <= 1);
		bool Can_HugCrush = (low_hp && allow_perform);

		float stamina = GetStaminaPercentage(giant);
		bool Can_Force = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles") && IsHostile(giant, tiny);

		if (Can_Force && crush_rng <= 1 && stamina >= 0.50) {
			return true;
		}
		if (Can_HugCrush) {
			return true;
		}
		return false;
	}

	void AI_HealOrShrink(Actor* giant, Actor* tiny, int rng) {
		bool hostile = IsHostile(giant, tiny);
		
		if (hostile || rng <= 1) { // chance to get drained by follower
			AnimationManager::StartAnim("Huggies_Shrink", giant);
			AnimationManager::StartAnim("Huggies_Shrink_Victim", tiny);
		} else { // else heal
			StartHealingAnimation(giant, tiny);
		}
	}

	void AI_SelectActionToPlay(Actor* pred, Actor* prey, int rng, int butt_rng, int action_rng) {
		if (IsGtsBusy(pred)) {
			log::info("{} Is Gts Busy", pred->GetDisplayFullName());
			return;
		}

		if (rng <= 2 && butt_rng <= 2) {
			AI_ButtCrush(pred, prey);
			return;
		} else if (rng <= 3) {
			AI_StrongStomp(pred, action_rng);
			return;
		} else if (rng <= 6) {
			AI_LightStomp(pred, action_rng);
			return;
		} else if (rng <= 8) {
			AI_Kicks(pred, action_rng);
			return;
		} else if (rng <= 9) {
			AI_Tramples(pred, action_rng);
			return;
		}
	}
}

namespace Gts {

	void AI_TryAction(Actor* actor) {
		float scale = std::clamp(get_visual_scale(actor), 1.0f, 6.0f);
		if (GetAV(actor, ActorValue::kHealth) < 0) {
			log::info("Action: {} Health is < 0", actor->GetDisplayFullName());
			return;
		}
		if (!IsGtsBusy(actor)) {
			int rng = rand() % 100;
			if (rng > 7 && rng < 33 * scale) {
				AI_DoStomp_Kick_ButtCrush(actor);
				return;
			} else if (rng > 3 && rng < 7) {
				AI_DoSandwich(actor);
				return;
			} else if (rng <= 3) {
				int HugsOrThigh = rand()% 10;
				if (HugsOrThigh > 5) {
					AI_DoHugs(actor);
				} else {
					AI_DoThighCrush(actor);
				}
			}
		}
		// Random Vore is managed inside Vore.cpp, RandomVoreAttempt(Actor* pred) function
	}

	void AI_DoStomp_Kick_ButtCrush(Actor* pred) {
		int rng = rand() % 10;
        int butt_rng = rand() % 10;
        int action_rng = rand() % 10;
        std::size_t amount = 6;
        std::vector<Actor*> preys = AiManager::GetSingleton().RandomStomp(pred, amount);
        for (auto prey: preys) {
            if (AiManager::GetSingleton().CanStomp(pred, prey)) {
                AI_SelectActionToPlay(pred, prey, rng, butt_rng, action_rng);
            }
        }
    }

	void AI_DoSandwich(Actor* pred) {
		if (!Persistent::GetSingleton().Sandwich_Ai || IsCrawling(pred)) {
			return;
		}
		auto& Sandwiching = ThighSandwichController::GetSingleton();

		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, 1);
		for (auto prey: preys) {
			if (CanPerformAnimationOn(pred, prey)) { // player check is done inside CanSandwich()
				Sandwiching.StartSandwiching(pred, prey);
				auto node = find_node(pred, "GiantessRune", false);
				if (node) {
					node->local.scale = 0.01;
					update_node(node);
				}
			}
		}
	}

	void AI_DoHugs(Actor* pred) {
		if (!Persistent::GetSingleton().Hugs_Ai || IsCrawling(pred)) {
			return;
		}
		int rng = rand() % 7;
		if (rng >= 5) {
			if (CanDoPaired(pred) && !IsSynced(pred) && !IsTransferingTiny(pred)) {
				auto& hugs = HugAnimationController::GetSingleton();
				std::size_t numberOfPrey = 1;
				std::vector<Actor*> preys = hugs.GetHugTargetsInFront(pred, numberOfPrey);
				for (auto prey: preys) {
					float sizedifference = GetSizeDifference(pred, prey, SizeType::VisualScale, false, true);
					if (sizedifference > Action_Hug && sizedifference < GetHugShrinkThreshold(pred)) {
						// ^ If Size > 0.92 (minimum) && Size < 2.5 + perk bonus (maximum) threshold basically
						AI_StartHugs(pred, prey);
					}
				}
			}
		}
	}

	void AI_StartHugs(Actor* pred, Actor* prey) {
		auto& hugging = HugAnimationController::GetSingleton();
		auto& persist = Persistent::GetSingleton();
		if (!hugging.CanHug(pred, prey)) {
			return;
		}
		if (!pred->IsInCombat() && persist.vore_combatonly) {
			return;
		}
		if (prey->formID != 0x14 && !IsHostile(pred, prey) && !IsTeammate(pred)) {
			return;
		}
		if (prey->formID == 0x14 && !persist.vore_allowplayervore) {
			return;
		}
		HugShrink::GetSingleton().HugActor(pred, prey);

		AnimationManager::StartAnim("Huggies_Try", pred);

		if (pred->IsSneaking() && !IsCrawling(pred)) {
			AnimationManager::StartAnim("Huggies_Try_Victim_S", prey); // GTSBEH_HugAbsorbStart_Sneak_V
		} else {
			AnimationManager::StartAnim("Huggies_Try_Victim", prey); //   GTSBEH_HugAbsorbStart_V
		}
		AI_StartHugsTask(pred, prey);
	}

	void AI_StartHugsTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("Huggies_Forced_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		static Timer ActionTimer = Timer(2.5);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			bool AllyHugged;
			bool IsDead = (tinyref->IsDead() || giantref->IsDead());
			tinyref->GetGraphVariableBool("GTS_IsFollower", AllyHugged);

			if (!HugShrink::GetHuggiesActor(giantref)) {
				if (!AllyHugged) {
					log::info("Ally isn't hugged, aborting and pushing");
					PushActorAway(giantref, tinyref, 1.0);
				}
				return false;
			}

			if (ActionTimer.ShouldRunFrame()) {
				int rng = rand() % 20;
				if (rng < 12) {
					if (!Runtime::HasPerkTeam(giantref, "HugCrush_LovingEmbrace")) {
						rng = 1; // always force crush and always shrink
					}	
					
					if (AI_CanHugCrush(giantref, tinyref, rng)) {
						AnimationManager::StartAnim("Huggies_HugCrush", giantref);
						AnimationManager::StartAnim("Huggies_HugCrush_Victim", tinyref);
					} else {
						AI_HealOrShrink(giant, tiny, rng);
					}
				}
			}
			if (IsDead) {
				return false;
			}
			return true;
		});
	}

	void AI_DoThighCrush(Actor* giant) {
		if (Persistent::GetSingleton().Thigh_Ai == false) {
			return;
		}
		std::vector<Actor*> tinies = ThighCrushController::GetSingleton().GetThighTargetsInFront(giant, 1, true);
		log::info("Starting Thigh Crush");
		if (!tinies.empty()) {
			Actor* tiny = tinies[0];
			if (tiny) {
				ThighCrushController::GetSingleton().StartThighCrush(giant, tiny, true);
			}
		}
	}

	void AI_StartThighCrushTask(Actor* giant) {
		std::string name = std::format("ThighCrush_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		float Start = Time::WorldTimeElapsed();
		static Timer ActionTimer = Timer(6.0);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			Actor* giantref = gianthandle.get().get();
			float Finish = Time::WorldTimeElapsed();

			if (Finish - Start > 0.10) {
				if (!IsThighCrushing(giantref)) {
					log::info("!ThighCrushing");
					return false;
				}

				if (ActionTimer.ShouldRunFrame()) {
					log::info("!CanThighCrush");

					bool ForceAbort = GetAV(giantref, ActorValue::kStamina) <= 2.0;
					DamageAV(giantref, ActorValue::kStamina, 0.025);

					if (ForceAbort) {
						log::info("Force Abort");
						AnimationManager::StartAnim("ThighLoopExit", giantref);
						return true;
					}

					std::vector<Actor*> targets = ThighCrushController::GetSingleton().GetThighTargetsInFront(giantref, 1, true);
					log::info("Seeking Targets");
					if (targets.empty()) {
						log::info("Is Empty");
						AnimationManager::StartAnim("ThighLoopExit", giantref);
						return true;
					} else if (!targets.empty() && !ThighCrushController::GetSingleton().CanThighCrush(giantref, targets[0], true)) {
						log::info("Can't Thigh Crush {}", targets[0]->GetDisplayFullName());
						AnimationManager::StartAnim("ThighLoopExit", giantref);
						return true;
					} else {
						log::info("Doing Thigh Attack");
						AnimationManager::StartAnim("ThighLoopAttack", giantref);
						return true;
					}
					return true;
				}
				return true;
			}
			return true;
		});
	}
}
