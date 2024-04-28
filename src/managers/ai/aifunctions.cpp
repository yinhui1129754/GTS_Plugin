#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/audio/footstep.hpp"
#include "utils/papyrusUtils.hpp"
#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace {
	float GetScareThreshold(Actor* giant) {
		float threshold = 2.5;
		if (giant->IsSneaking()) { // If we sneak/prone/crawl = make threshold bigger so it's harder to scare actors
			threshold += 0.8;
		}
		if (IsCrawling(giant)) {
			threshold += 1.45;
		}
		if (IsProning(giant)) {
			threshold += 1.45;
		}
		if (giant->AsActorState()->IsWalking()) { // harder to scare if we're approaching slowly
			threshold *= 1.35;
		}
		if (giant->IsRunning()) { // easier to scare
			threshold *= 0.75;
		}
		return threshold;
	}
}

namespace Gts {
	void Task_InitHavokTask(Actor* tiny) {

		float startTime = Time::WorldTimeElapsed();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		std::string taskname = std::format("EnterRagdoll_{}", tiny->formID);

		TaskManager::Run(taskname, [=](auto& update){
			if (!tinyHandle) {
				return false;
			}
			Actor* tinyref = tinyHandle.get().get();
			if (!tinyref) {
				return false;
			}
			if (!tinyref->IsDead()) {
				return false;
			}
			double endTime = Time::WorldTimeElapsed();

			if ((endTime - startTime) > 0.05) {
				tinyref->InitHavok(); // Hopefully will fix occasional Ragdoll issues
				return false;
			} 
			return true;
		});
	}
	
	void KillActor(Actor* giant, Actor* tiny) {
		if (!tiny->IsDead()) {
			StartCombat(tiny, giant);
		}
		float hp = GetMaxAV(tiny, ActorValue::kHealth) * 3.0;	
		InflictSizeDamage(giant, tiny, hp); // just to make sure
		
		if (tiny->formID == 0x14) {
			tiny->KillImpl(giant, 1, true, true);
			tiny->SetAlpha(0.0);
		}
		auto* eventsource = ScriptEventSourceHolder::GetSingleton();
		if (eventsource) {
			auto event = TESDeathEvent();
			event.actorDying = skyrim_cast<TESObjectREFR*>(tiny)->CreateRefHandle().get();
			event.actorKiller = skyrim_cast<TESObjectREFR*>(giant)->CreateRefHandle().get();
			event.dead = true;
			eventsource->SendEvent(&event);
		}

		Task_InitHavokTask(tiny);
	}

	// butt crush related things

	float GetGrowthCount(Actor* giant) {
		auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			return transient->ButtCrushGrowthAmount;
		}
		return 1.0;
	}

	float GetGrowthLimit(Actor* actor) {
		float limit = 0;
		if (Runtime::HasPerkTeam(actor, "ButtCrush_GrowingDisaster")) {
			limit += 3.0;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
			limit += 4.0;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
			limit += 5.0;
		}
		return limit;
	}

	float GetButtCrushDamage(Actor* actor) {
		float damage = 1.0;
		if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
			damage += 0.30;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
			damage += 0.70;
		}
		return damage;
	}

	void ModGrowthCount(Actor* giant, float value, bool reset) {
		auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			transient->ButtCrushGrowthAmount += value;
			if (reset) {
				transient->ButtCrushGrowthAmount = 0.0;
			}
		}
	}

	void SetButtCrushSize(Actor* giant, float value, bool reset) {
		auto saved_data = Transient::GetSingleton().GetData(giant);
		if (saved_data) {
			saved_data->buttcrush_max_size += value;
			if (reset) {
				float remove_size = (saved_data->buttcrush_max_size / get_natural_scale(giant, false)) * game_getactorscale(giant);
				update_target_scale(giant, -remove_size, SizeEffectType::kNeutral);
				if (get_target_scale(giant) < get_natural_scale(giant, true)) {
					set_target_scale(giant, get_natural_scale(giant, true)); // Protect against going into negatives
				}
				saved_data->buttcrush_max_size = 0;
			}
		}
	}

	float GetButtCrushSize(Actor* giant) {
		auto saved_data = Transient::GetSingleton().GetData(giant);
		if (saved_data) {
			float butt_crush_size = std::clamp(saved_data->buttcrush_max_size, 0.0f, 1000000.0f);
			return butt_crush_size;
		}
		return 0.0;
	}

	void ForceFlee(Actor* giant, Actor* tiny, float duration) {
		float oldConfidence = GetAV(tiny, ActorValue::kConfidence);

		float Start = Time::WorldTimeElapsed();
		std::string name = std::format("ScareAway_{}", tiny->formID);
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		ActorHandle giantHandle = giant->CreateRefHandle();

		duration *= GetSizeDifference(giant, tiny, SizeType::VisualScale, false, true);

		SetAV(tiny, ActorValue::kConfidence, 0.0);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!tinyHandle) {
				return false;
			}
			if (!giantHandle) {
				return false;
			}
			float Finish = Time::WorldTimeElapsed();

			auto tinyRef = tinyHandle.get().get();
			auto giantRef = giantHandle.get().get();

			if (tinyRef->IsDead()) {
				SetAV(tinyRef, ActorValue::kConfidence, oldConfidence);
				return false; // To be safe
			}

			ApplyActionCooldown(tinyRef, CooldownSource::Action_ScareOther);

			float timepassed = Finish - Start;
			if (IsMoving(tinyRef)) {
				int FallChance = rand() % 6000;// Chance to Trip
				if (FallChance <= 2 && !IsRagdolled(tinyRef)) {
					PushActorAway(giantRef, tinyRef, 1.0);
				}
			}
			
			if (timepassed >= duration) {
				SetAV(tinyRef, ActorValue::kConfidence, oldConfidence);
				return false; // end it
			}
			return true;
		});
	}

	void ScareActors(Actor* giant) {
		auto profiler = Profilers::Profile("ActorUtils: ScareActors");
		if (!Persistent::GetSingleton().actors_panic) {
			return; // Disallow Panic if bool is false.
		}
		for (auto tiny: FindSomeActors("AiActors", 2)) {
			if (tiny != giant && tiny->formID != 0x14 && !IsTeammate(tiny)) {
				if (tiny->IsDead()) {
					return;
				}
				if (IsBeingHeld(giant, tiny)) {
					return;
				}
				float get_difference = GetSizeDifference(giant, tiny, SizeType::VisualScale, false, true); // Apply HH difference as well
				float sizedifference = std::clamp(get_difference, 0.10f, 12.0f);

				float distancecheck = 128.0 * GetMovementModifier(giant);
				float threshold = GetScareThreshold(giant);
				if (sizedifference >= threshold) {
					NiPoint3 GiantDist = giant->GetPosition();
					NiPoint3 ObserverDist = tiny->GetPosition();
					float distance = (GiantDist - ObserverDist).Length();

					if (distance <= distancecheck * sizedifference) {
						auto combat = tiny->GetActorRuntimeData().combatController;

						tiny->GetActorRuntimeData().currentCombatTarget = giant->CreateRefHandle();
						auto TinyRef = skyrim_cast<TESObjectREFR*>(tiny);

						if (TinyRef) {
							auto GiantRef = skyrim_cast<TESObjectREFR*>(giant);
							if (GiantRef) {
								bool SeeingOther;
								bool IsTrue = tiny->HasLineOfSight(GiantRef, SeeingOther);
								if (IsTrue || distance < (distancecheck/1.5) * sizedifference) {
									auto cell = tiny->GetParentCell();
									if (cell) {
										if (!combat) {
											tiny->InitiateFlee(TinyRef, true, true, true, cell, TinyRef, 100.0, 465.0 * sizedifference);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
