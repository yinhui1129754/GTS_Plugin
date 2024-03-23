#include "managers/damage/CollisionDamage.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
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

		tiny->InitHavok(); // Hopefully will fix occasional Ragdoll issues
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
			limit += 2.0;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
			limit += 3.0;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
			limit += 4.0;
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

	void SetBonusSize(Actor* giant, float value, bool reset) {
		auto saved_data = Persistent::GetSingleton().GetData(giant);
		if (saved_data) {
			saved_data->bonus_max_size += value;
			if (reset) {
				SpringGrow_Free(giant, -saved_data->bonus_max_size, 0.3 / GetAnimationSlowdown(giant), "SizeReset");
				///update_target_scale(giant, -saved_data->bonus_max_size, SizeEffectType::kNeutral);
				if (get_target_scale(giant) < get_natural_scale(giant)) {
					set_target_scale(giant, get_natural_scale(giant)); // Protect against going into negatives
				}
				saved_data->bonus_max_size = 0;
			}
		}
	}

	void InitiateFlee(Actor* tiny) {

		TESForm* Flee_From_Form = TESForm::LookupByID<TESForm>(0x000197F1);
		TESForm* Flee_To_Form = TESForm::LookupByID<TESForm>(0x000C7039);
		if (Flee_From_Form) {
			log::info("Flee To found!");
			TESPackage* FleeFrom = skyrim_cast<TESPackage*>(Flee_From_Form);
			if (FleeFrom) {
				log::info("Flee From: True");
				tiny->GetActorBase()->aiPackages.packages.push_front(FreeFrom);
				//tiny->PutCreatedPackage(FleeFrom, true, false, true); 
			}
		}

		if (Flee_To_Form) {
			log::info("Flee From found, trying cast");
			TESPackage* FleeTo = skyrim_cast<TESPackage*>(Flee_To_Form);
			if (FleeTo) {
				log::info("Flee To: True");
				//tiny->GetActorBase().aiPackages.packages.push_front(FreeTo);
			}

			//tiny->PutCreatedPackage(FleeTo, true, false, true); 
			log::info("Putting existing package");
		}
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
				float GiantScale = get_visual_scale(giant);
				float TinyScale = get_visual_scale(tiny);
				float sizedifference = std::clamp(GiantScale/TinyScale, 0.10f, 12.0f);
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
