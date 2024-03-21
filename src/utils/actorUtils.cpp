#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/TinyCalamity_Shrink.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/animation/Grab.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "utils/DeathReport.hpp"
#include "managers/highheel.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "colliders/actor.hpp"
#include "managers/Rumble.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "utils/looting.hpp"
#include "scale/height.hpp"
#include "data/runtime.hpp"
#include "colliders/RE.hpp"
#include "rays/raycast.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "utils/debug.hpp"
#include "utils/av.hpp"
#include "profiler.hpp"
#include "events.hpp"
#include "spring.hpp"
#include "timer.hpp"

#include "node.hpp"

#include <vector>
#include <string>

using namespace RE;
using namespace Gts;

namespace {
	const float EPS = 1e-4;

	float GetGrowthReduction(float size) {
		// https://www.desmos.com/calculator/pqgliwxzi2
		SoftPotential cut {
			.k = 1.08,
			.n = 0.90,
			.s = 3.00,
			.a = 0.0,
		};
		float power = soft_power(size, cut);
		if (SizeManager::GetSingleton().BalancedMode() >= 2.0) {
			return std::clamp(power, 1.0f, 99999.0f); // So it never reports values below 1.0. Just to make sure.
		} else {
			return 1.0;
		}
	}

	bool Utils_ManageTinyProtection(Actor* giantref, bool force_cancel, bool Balance) {
		float sp = GetAV(giantref, ActorValue::kStamina);

		if (!force_cancel && Balance) {
			float perk = Perk_GetCostReduction(giantref);
			float damage = 0.12 * TimeScale() * perk;
			if (giantref->formID != 0x14) {
				damage *= 0.5; // less stamina drain for NPC's
			}
			DamageAV(giantref, ActorValue::kStamina, damage);
		}

		if (sp <= 1.0 || force_cancel) {
			float OldScale;
			giantref->GetGraphVariableFloat("GiantessScale", OldScale); // save old scale
			giantref->SetGraphVariableFloat("GiantessScale", 1.0); // Needed to allow Stagger to play, else it won't work

			if (!force_cancel) {
				StaggerActor(giantref, 0.25);
			}
			float scale = get_visual_scale(giantref);

			StaggerActor_Around(giantref, 48.0);

			auto node = find_node(giantref, "NPC Root [Root]");
			Runtime::PlaySoundAtNode("Magic_BreakTinyProtection", giantref, 1.0, 1.0, "NPC COM [COM ]");
			
			if (node) {
				NiPoint3 position = node->world.translate;

				std::string name_com = std::format("BreakProtect_{}", giantref->formID);
				std::string name_root = std::format("BreakProtect_Root_{}", giantref->formID);

				GRumble::Once(name_com, giantref, 8.6, 0.20, "NPC COM [COM ]");
				GRumble::Once(name_root, giantref, 8.6, 0.20, "NPC Root [Root]");

				SpawnParticle(giantref, 6.00, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), position, scale * 3.4, 7, nullptr); // Spawn it
			}
			giantref->SetGraphVariableFloat("GiantessScale", OldScale);

			return false;
		}
		return true;
	}

	float GetPerkBonus_OnTheEdge(Actor* giant, float amt) {
		float bonus = 1.0;
		bool perk = Runtime::HasPerkTeam(giant, "OnTheEdge");
		if (perk) {
			float GetHP = std::clamp(GetHealthPercentage(giant) + 0.4f, 0.5f, 1.0f); // Bonus Size Gain if Actor has perk
			if (amt > 0) {
				bonus /= GetHP;
			} else if (amt < 0) {
				bonus *= GetHP;
			} // When health is < 60%, empower growth by up to 50%. Max value at 10% health.
		}
		return bonus;
	}

	float ShakeStrength(Actor* Source) {
		float Size = get_visual_scale(Source);
		float k = 0.065;
		float n = 1.0;
		float s = 1.12;
		float Result = 1.0/(pow(1.0+pow(k*(Size),n*s),1.0/s));
		return Result;
	}

	void Task_AdjustHalfLifeTask(Actor* tiny, float halflife) {
		auto& Persist = Persistent::GetSingleton();
		auto actor_data = Persist.GetData(tiny);
		float old_halflife = 0.0;
		if (actor_data) {
			old_halflife = actor_data->half_life; // record old half life
			actor_data->half_life = halflife;
		}

		auto Start = Time::FramesElapsed();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		std::string name = std::format("AdjustHalfLife_{}", tiny->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!tinyhandle) {
				return false;
			}
			auto tinyref = tinyhandle.get().get();
			float timepassed = Time::FramesElapsed() - Start;
			if (timepassed > 1.2) {
				if (actor_data) {
					actor_data->half_life = old_halflife;
				}
				return false;
			}

			return true;
		});
	}

	ExtraDataList* CreateExDataList() {
		size_t a_size;
		if (SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) && (REL::Module::get().version() >= SKSE::RUNTIME_SSE_1_6_629)) {
			a_size = 0x20;
		} else {
			a_size = 0x18;
		}
		auto memory = RE::malloc(a_size);
		std::memset(memory, 0, a_size);
		if (SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) && (REL::Module::get().version() >= SKSE::RUNTIME_SSE_1_6_629)) {
			// reinterpret_cast<std::uintptr_t*>(memory)[0] = a_vtbl; // Unknown vtable location add once REd
			REL::RelocateMember<BSReadWriteLock>(memory, 0x18) = BSReadWriteLock();
		} else {
			REL::RelocateMember<BSReadWriteLock>(memory, 0x10) = BSReadWriteLock();
		}
		return static_cast<ExtraDataList*>(memory);
	}

	struct SpringGrowData {
		Spring amount = Spring(0.0, 1.0);
		float addedSoFar = 0.0;
		ActorHandle actor;

		SpringGrowData(Actor* actor, float amountToAdd, float halfLife) : actor(actor->CreateRefHandle()) {
			amount.value = 0.0;
			amount.target = amountToAdd;
			amount.halflife = halfLife;
		}
	};

	struct SpringShrinkData {
		Spring amount = Spring(0.0, 1.0);
		float addedSoFar = 0.0;
		ActorHandle actor;

		SpringShrinkData(Actor* actor, float amountToAdd, float halfLife) : actor(actor->CreateRefHandle()) {
			amount.value = 0.0;
			amount.target = amountToAdd;
			amount.halflife = halfLife;
		}
	};
}

RE::ExtraDataList::~ExtraDataList() {
}

namespace Gts {
	RE::NiPoint3 RotateAngleAxis(const RE::NiPoint3& vec, const float angle, const RE::NiPoint3& axis) {
		float S = sin(angle);
		float C = cos(angle);

		const float XX = axis.x * axis.x;
		const float YY = axis.y * axis.y;
		const float ZZ = axis.z * axis.z;

		const float XY = axis.x * axis.y;
		const float YZ = axis.y * axis.z;
		const float ZX = axis.z * axis.x;

		const float XS = axis.x * S;
		const float YS = axis.y * S;
		const float ZS = axis.z * S;

		const float OMC = 1.f - C;

		return RE::NiPoint3(
			(OMC * XX + C) * vec.x + (OMC * XY - ZS) * vec.y + (OMC * ZX + YS) * vec.z,
			(OMC * XY + ZS) * vec.x + (OMC * YY + C) * vec.y + (OMC * YZ - XS) * vec.z,
			(OMC * ZX - YS) * vec.x + (OMC * YZ + XS) * vec.y + (OMC * ZZ + C) * vec.z
			);
	}

	Actor* GetActorPtr(Actor* actor) {
		return actor;
	}

	Actor* GetActorPtr(Actor& actor) {
		return &actor;
	}

	Actor* GetActorPtr(ActorHandle& actor) {
		if (!actor) {
			return nullptr;
		}
		return actor.get().get();
	}
	Actor* GetActorPtr(const ActorHandle& actor) {
		if (!actor) {
			return nullptr;
		}
		return actor.get().get();
	}
	Actor* GetActorPtr(FormID formId) {
		Actor* actor = TESForm::LookupByID<Actor>(formId);
		if (!actor) {
			return nullptr;
		}
		return actor;
	}

	Actor* GetCharContActor(bhkCharacterController* charCont) {
		for (auto actor: find_actors()) {
			if (charCont == actor->GetCharController()) {
				return actor;
			}
		}
		// Sadly feels like that's the only reliable way to get the actor
		// Possible other method (complex, Meh321 from RE Discord):
		// - hook ctor and dtor of the bhkCharacterController, there's more info in the constructor 
		//  (like which object root node it's created for) and store custom formid -> ptr lookup (or other way around) 
		//  from which you can get it later
		//  bhkCharProxyControllerCinfo+0x10 is root NiNode* of TESObjectREFR
		return nullptr;
	}

	float GetLaunchPower(Actor* giant, float sizeRatio) {
		// https://www.desmos.com/calculator/wh0vwgljfl
		SoftPotential launch {
			.k = 1.42,
			.n = 0.78,
			.s = 0.6,
			.a = 0.8,
		};
		float power = soft_power(sizeRatio, launch);

		if (!giant) {
			return 1.0;
		}

		float limit = (6.0 * get_visual_scale(giant));

		if (power > limit) {
			return limit;
		}
		
		return power;
	}

	void StartResetTask(Actor* tiny) {
		std::string name = std::format("ResetActor_{}", tiny->formID);
		float Start = Time::WorldTimeElapsed();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!tinyhandle) {
				return false;
			}
			auto tiny = tinyhandle.get().get();
			float Finish = Time::WorldTimeElapsed();
			float timepassed = Finish - Start;
			if (timepassed < 1.0) {
				return true; // not enough time has passed yet
			}
			EventDispatcher::DoResetActor(tiny);
			return false; // stop task, we reset the actor
		});
	}

	void PlayMoanSound(Actor* actor, float volume) {
		if (IsFemale(actor) && IsHuman(actor)) {
			float falloff = 0.9 * get_visual_scale(actor);
			Runtime::PlaySoundAtNode_FallOff("MoanSound", actor, volume, 1.0, "NPC Head [Head]", falloff);
		}
	}

	void PlayLaughSound(Actor* actor, float volume, int type) {
		float falloff = 0.11 * get_visual_scale(actor);
		if (IsFemale(actor) && IsHuman(actor)) {
			if (type == 2) {
				Runtime::PlaySoundAtNode_FallOff("LaughSound_Type2", actor, volume, 1.0, "NPC Head [Head]", falloff);
			} else {
				Runtime::PlaySoundAtNode_FallOff("LaughSound_Type1", actor, volume, 1.0, "NPC Head [Head]", falloff);
			}
		}
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 G T S   ST A T E S  B O O L S                                                                      //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool BehaviorGraph_DisableHH(Actor* actor) { // should .dll disable HH if Behavior Graph has HH Disable data?
		bool disable = false;
		actor->GetGraphVariableBool("GTS_DisableHH", disable);
		if (actor->formID == 0x14 && IsFirstPerson()) {
			return false;
		}
		bool anims = AnimationsInstalled(actor);
		if (!anims) {
			return false; // prevent hh from being disabled if there's no Nemesis Generation
		}

		return disable;
	}

	bool IsEquipBusy(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: IsEquipBusy");
		int State;
		actor->GetGraphVariableInt("currentDefaultState", State);
		if (State >= 10 && State <= 20) {
			return true;
		}
		return false;
	}

	bool IsRagdolled(Actor* actor) {
		bool ragdoll = actor->IsInRagdollState();
		return ragdoll;
	}

	bool IsChangingSize(Actor* actor) { // Used to disallow growth/shrink during specific animations
		bool Growing = false;
		bool Shrinking = false;
		actor->GetGraphVariableBool("GTS_IsGrowing", Growing);
		actor->GetGraphVariableBool("GTS_IsShrinking", Shrinking);

		return Growing || Shrinking;
	}

	bool IsProning(Actor* actor) {
		bool prone = false;
		auto transient = Transient::GetSingleton().GetData(actor);
		actor->GetGraphVariableBool("GTS_IsProne", prone);
		if (actor->formID == 0x14 && actor->IsSneaking() && IsFirstPerson() && transient) {
			return transient->FPProning; // Because we have no FP behaviors, 
			// ^ it is Needed to fix proning being applied to FP even when Prone is off
		}
		return actor!= nullptr && prone;
	}

	bool IsCrawling(Actor* actor) {
		bool crawl = false;
		auto transient = Transient::GetSingleton().GetData(actor);
		actor->GetGraphVariableBool("GTS_IsCrawling", crawl);
		if (actor->formID == 0x14 && actor->IsSneaking() && IsFirstPerson() && transient) {
			return transient->FPCrawling; // Needed to fix crawling being applied to FP even when Prone is off
		}
		return actor!= nullptr && actor->IsSneaking() && crawl;
	}

	bool IsInBalanceMode() {
		return SizeManager::GetSingleton().BalancedMode() > 1.0;
	}

	bool IsHugCrushing(Actor* actor) {
		bool IsHugCrushing;
		actor->GetGraphVariableBool("IsHugCrushing", IsHugCrushing);
		return IsHugCrushing;
	}

	bool IsHugHealing(Actor* actor) {
		bool IsHugHealing;
		actor->GetGraphVariableBool("GTS_IsHugHealing", IsHugHealing);
		return IsHugHealing;
	}

	bool IsHuggingFriendly(Actor* actor) {
		bool friendly;
		actor->GetGraphVariableBool("GTS_IsFollower", friendly);
		return friendly;
	}

	bool IsTransitioning(Actor* actor) { // reports sneak transition to crawl
		bool transition = false;
		actor->GetGraphVariableBool("GTS_Transitioning", transition);
		return transition;
	}

	bool IsFootGrinding(Actor* actor) {
		bool grind;
		actor->GetGraphVariableBool("GTS_IsFootGrinding", grind);
		return grind;
	}

	bool isTrampling(Actor* actor) {
		bool trample;
		actor->GetGraphVariableBool("GTS_IsTrampling", trample);
		return trample;
	}

	bool IsJumping(Actor* actor) {
		bool jumping = false;
		actor->GetGraphVariableBool("bInJumpState", jumping);
		return jumping;
	}

	bool IsBeingHeld(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			return transient->being_held && !tiny->IsDead();
		}
		return false;
	}

	bool IsBetweenBreasts(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->between_breasts;
		}
		return false;
	}

	bool IsTransferingTiny(Actor* actor) { // Reports 'Do we have someone grabed?'
		int grabbed;
		actor->GetGraphVariableInt("GTS_GrabbedTiny", grabbed);
		return grabbed > 0;
	}

	bool IsUsingThighAnimations(Actor* actor) { // Do we currently use Thigh Crush / Thigh Sandwich?
		int sitting;
		actor->GetGraphVariableInt("GTS_Sitting", sitting);
		return sitting > 0;
	}

	bool IsSynced(Actor* actor) {
		bool sync;
		actor->GetGraphVariableBool("bIsSynced", sync);
		return sync;
	}

	bool CanDoPaired(Actor* actor) {
		bool paired;
		actor->GetGraphVariableBool("GTS_CanDoPaired", paired);
		return paired;
	}


	bool IsThighCrushing(Actor* actor) { // Are we currently doing Thigh Crush?
		int crushing;
		actor->GetGraphVariableInt("GTS_IsThighCrushing", crushing);
		return crushing > 0;
	}

	bool IsThighSandwiching(Actor* actor) { // Are we currently Thigh Sandwiching?
		int sandwiching;
		actor->GetGraphVariableInt("GTS_IsThighSandwiching", sandwiching);
		return sandwiching > 0;
	}

	bool IsStomping(Actor* actor) {
		int Stomping;
		actor->GetGraphVariableInt("GTS_IsStomping", Stomping);
		return Stomping > 0;
	}

	bool IsBeingEaten(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			return transient->about_to_be_eaten;
		}
		return false;
	}

	bool IsGtsBusy(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: IsGtsBusy"); 
		bool GTSBusy;
		actor->GetGraphVariableBool("GTS_Busy", GTSBusy);

		bool Busy = GTSBusy && !CanDoCombo(actor);
		return Busy;
	}

	bool CanDoCombo(Actor* actor) {
		bool Combo;
		actor->GetGraphVariableBool("GTS_CanCombo", Combo);
		return Combo;
	}

	bool IsCameraEnabled(Actor* actor) {
		bool Camera;
		actor->GetGraphVariableBool("GTS_VoreCamera", Camera);
		return Camera;
	}

	bool IsCrawlVoring(Actor* actor) {
		bool Voring;
		actor->GetGraphVariableBool("GTS_IsCrawlVoring", Voring);
		return Voring;//Voring;
	}

	bool IsButtCrushing(Actor* actor) {
		bool ButtCrushing;
		actor->GetGraphVariableBool("GTS_IsButtCrushing", ButtCrushing);
		return ButtCrushing;
	}

	bool ButtCrush_IsAbleToGrow(Actor* actor, float limit) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->ButtCrushGrowthAmount < limit;
		}
		return false;
	}

	bool IsBeingGrinded(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		bool grinded = false;
		actor->GetGraphVariableBool("GTS_BeingGrinded", grinded);
		if (transient) {
			return transient->being_foot_grinded;
		}
		return grinded;
	}

	bool IsHugging(Actor* actor) {
		bool hugging = false;
		actor->GetGraphVariableBool("GTS_Hugging", hugging);
		return hugging;
	}

	bool IsBeingHugged(Actor* actor) {
		bool hugged = false;
		actor->GetGraphVariableBool("GTS_BeingHugged", hugged);
		return hugged;
	}

	bool CanDoButtCrush(Actor* actor, bool apply_cooldown) {
		bool Allow = IsActionOnCooldown(actor, CooldownSource::Action_ButtCrush);

		if (!Allow && apply_cooldown) { // send it to cooldown if it returns 'not busy'
			ApplyActionCooldown(actor, CooldownSource::Action_ButtCrush);
		}

		return !Allow; // return flipped OnCooldown. By default it false, we flip it so it returns True (Can perform butt crush)
	}

	bool GetCameraOverride(Actor* actor) {
		if (actor->formID == 0x14) {
			auto transient = Transient::GetSingleton().GetData(actor);
			if (transient) {
				return transient->OverrideCamera;
			}
			return false;
		}
		return false;
	}



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 G T S   ST A T E S  O T H E R                                                                      //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	bool IsGrowthSpurtActive(Actor* actor) {
		if (!Runtime::HasPerkTeam(actor, "GrowthOfStrength")) {
			return false;
		}
		if (HasGrowthSpurt(actor)) {
			return true;
		}
		return false;
	}

	bool HasGrowthSpurt(Actor* actor) {
		bool Growth1 = Runtime::HasMagicEffect(actor, "GrowthSpurt1");
		bool Growth2 = Runtime::HasMagicEffect(actor, "GrowthSpurt2");
		bool Growth3 = Runtime::HasMagicEffect(actor, "GrowthSpurt3");
		if (Growth1 || Growth2 || Growth3) {
			return true;
		} else {
			return false;
		}
	}

	bool InBleedout(Actor* actor) {
		return actor->AsActorState()->IsBleedingOut();
	}

	bool AllowStagger(Actor* giant, Actor* tiny) {
		if (Persistent::GetSingleton().allow_stagger == true) {
			return true; // Allow it
		} else if (Persistent::GetSingleton().allow_stagger == false && (giant->formID == 0x14 || IsTeammate(giant)) && (tiny->formID == 0x14 || IsTeammate(tiny))) {
			return false; // Protect
		}
		return true;
	}

	bool IsMechanical(Actor* actor) {
		bool dwemer = Runtime::HasKeyword(actor, "DwemerKeyword");
		return dwemer;
	}

	bool IsHuman(Actor* actor) { // Check if Actor is humanoid or not. Currently used for Hugs Animation and for playing moans
		bool vampire = Runtime::HasKeyword(actor, "VampireKeyword");
		bool dragon = Runtime::HasKeyword(actor, "DragonKeyword");
		bool animal = Runtime::HasKeyword(actor, "AnimalKeyword");
		bool dwemer = Runtime::HasKeyword(actor, "DwemerKeyword");
		bool undead = Runtime::HasKeyword(actor, "UndeadKeyword");
		bool creature = Runtime::HasKeyword(actor, "CreatureKeyword");
		if (!dragon && !animal && !dwemer && !undead && !creature) {
			return true; // Detect non-vampire
		} if (!dragon && !animal && !dwemer && !creature && undead && vampire) {
			return true; // Detect Vampire
		} else {
			return false;
		}
		return false;
	}

	bool IsBlacklisted(Actor* actor) {
		bool blacklist = Runtime::HasKeyword(actor, "BlackListKeyword");
		return blacklist;
	}

	bool IsInsect(Actor* actor, bool performcheck) {
		bool Check = Persistent::GetSingleton().AllowInsectVore;
		if (performcheck && Check) {
			return false;
		}
		bool Spider = Runtime::IsRace(actor, "FrostbiteSpiderRace");
		bool SpiderGiant = Runtime::IsRace(actor, "FrostbiteSpiderRaceGiant");
		bool SpiderLarge = Runtime::IsRace(actor, "FrostbiteSpiderRaceLarge");
		bool ChaurusReaper = Runtime::IsRace(actor, "ChaurusReaperRace");
		bool Chaurus = Runtime::IsRace(actor, "ChaurusRace");
		bool ChaurusHunterDLC = Runtime::IsRace(actor, "DLC1ChaurusHunterRace");
		bool ChaurusDLC = Runtime::IsRace(actor, "DLC1_BF_ChaurusRace");
		bool ExplSpider = Runtime::IsRace(actor, "DLC2ExpSpiderBaseRace");
		bool ExplSpiderPackMule = Runtime::IsRace(actor, "DLC2ExpSpiderPackmuleRace");
		bool AshHopper = Runtime::IsRace(actor, "DLC2AshHopperRace");
		if (Spider||SpiderGiant||SpiderLarge||ChaurusReaper||Chaurus||ChaurusHunterDLC||ChaurusDLC||ExplSpider||ExplSpiderPackMule||AshHopper) {
			return true;
		} else {
			return false;
		}
		return false;
	}

	bool IsFemale(Actor* actor) {
		auto base = actor->GetActorBase();
		int sex = 0;
		if (base) {
			if (base->GetSex()) {
				sex = base->GetSex();
			}
		}
		return sex == 1; // Else return sex value
	}

	bool IsDragon(Actor* actor) {
		if (Runtime::HasKeyword(actor, "DragonKeyword")) {
			return true;
		}
		if (Runtime::IsRace(actor, "dragonRace")) {
			return true;
		} 
		return false;
	}

	bool IsGiant(Actor* actor) {
		return Runtime::IsRace(actor, "GiantRace");
	}

	bool IsMammoth(Actor* actor) {
		return Runtime::IsRace(actor, "MammothRace");
	}

	bool IsLiving(Actor* actor) {
		bool IsDraugr = Runtime::HasKeyword(actor, "UndeadKeyword");
		bool IsDwemer = Runtime::HasKeyword(actor, "DwemerKeyword");
		bool IsVampire = Runtime::HasKeyword(actor, "VampireKeyword");
		if (IsVampire) {
			return true;
		}
		if (IsDraugr || IsDwemer) {
			return false;
		} else {
			return true;
		}
		return true;
	}

	bool IsUndead(Actor* actor, bool PerformCheck) {
		bool IsDraugr = Runtime::HasKeyword(actor, "UndeadKeyword");
		bool Check = Persistent::GetSingleton().AllowUndeadVore;
		if (Check && PerformCheck) {
			return false;
		}
		return IsDraugr;
	}

	bool WasReanimated(Actor* actor) { // must be called while actor is still alive, else it will return false.
		bool reanimated = false;
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			reanimated = transient->WasReanimated;
		}
		return reanimated;
	}

	bool IsHeadtracking(Actor* giant) { // Used to report True when we lock onto something, should be Player Exclusive.
		//Currently used to fix TDM mesh issues when we lock on someone.
		//auto profiler = Profilers::Profile("ActorUtils: HeadTracking");
		bool tracking;
		if (giant->formID == 0x14) {
			giant->GetGraphVariableBool("TDM_TargetLock", tracking); // get HT value, requires newest versions of TDM to work properly
		} else {
			tracking = false;
		}
		return tracking;
	}

	bool IsHostile(Actor* giant, Actor* tiny) {
		return tiny->IsHostileToActor(giant);
	}

	bool CanPerformAnimationOn(Actor* giant, Actor* tiny) {
		
		bool Busy = IsBeingGrinded(tiny) || IsBeingHugged(tiny) || IsHugging(giant);
		// If any of these is true = we disallow animation

		bool Teammate = IsTeammate(tiny);
		bool essential = IsEssential(tiny); // Teammate check is done here
		bool hostile = IsHostile(giant, tiny);
		bool no_protection = Persistent::GetSingleton().FollowerInteractions;
		bool Ignore_Protection = (giant->formID == 0x14 && Runtime::HasPerk(giant, "HugCrush_LovingEmbrace"));
		bool allow_teammate = (giant->formID != 0x14 && no_protection && IsTeammate(tiny) && IsTeammate(giant));

		if (Busy) {
			return false;
		}
		if (Ignore_Protection) {
			return true;
		}
		if (allow_teammate) { // allow if type is (teammate - teammate), and if bool is true
			return true;
		} else if (essential) { // disallow to perform on essentials
			return false;
		} else if (hostile) { // always allow for non-essential enemies. Will return true if Teammate is hostile towards someone (even player)
			return true;
		} else if (!Teammate) { // always allow for non-teammates
			return true;
		} else {
			return true; // else allow
		}
	}

	bool IsEssential(Actor* actor) {
		bool essential = actor->IsEssential() && Runtime::GetBool("ProtectEssentials");
		bool teammate = IsTeammate(actor);
		bool protectfollowers = Persistent::GetSingleton().FollowerProtection;
		if (actor->formID == 0x14) {
			return false; // we don't want to make the player immune
		} if (!teammate && essential) {
			return true;
		} else if (teammate && protectfollowers) {
			if (IsHostile(PlayerCharacter::GetSingleton(), actor)) {
				return false;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}

	bool AnimationsInstalled(Actor* giant) {
		bool installed = false;
		giant->GetGraphVariableBool("GTS_Installed", installed);
		return installed;
	}

	bool IsInGodMode(Actor* giant) {
		if (giant->formID != 0x14) {
			return false;
		}
		REL::Relocation<bool*> singleton{ RELOCATION_ID(517711, 404238) };
		return *singleton;
	}

	bool IsFreeCameraEnabled() {
		bool tfc = false;
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			if (camera->IsInFreeCameraMode()) {
				tfc = true;
			}
		}
		return tfc;
	}

	bool SizeRaycastEnabled() {
		return Persistent::GetSingleton().SizeRaycast_Enabled;
	}

	bool IsDebugEnabled() {
		return Runtime::GetBool("EnableDebugOverlay"); // used for debug mode of collisions and such
	}

	void ControlAnother(Actor* target, bool reset) {
		Actor* player = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetData(player);
		if (transient) {
			if (reset && target->formID == 0x14) {
				if (target->IsInControl) {
					DisableActorControls(target->IsInControl, true);
				}
				transient->IsInControl = nullptr;
				log::info("Controlled actor reset");
				return;
			} else {
				transient->IsInControl = target;
				DisableActorControls(target->IsInControl, false);
			}
		}
	}

	void DisableActorControls(Actor* giant, bool Reset) {
		auto RuntimeData = giant->GetActorRuntimeData();
		if (!Reset) {
			RuntimeData.boolFlags.set(BOOL_FLAGS::kAttackingDisabled);
			RuntimeData.boolFlags.set(BOOL_FLAGS::kCastingDisabled);
			RuntimeData.boolFlags.set(BOOL_FLAGS::kMovementBlocked);
			log::info("Actor Control disabled");
		} else {
			RuntimeData.boolFlags.reset(BOOL_FLAGS::kAttackingDisabled);
			RuntimeData.boolFlags.reset(BOOL_FLAGS::kCastingDisabled);
			RuntimeData.boolFlags.reset(BOOL_FLAGS::kMovementBlocked);
			log::info("Actor Control Enabled");
		}
		
	}

	Actor* GetPlayerOrControlled() {
		Actor* controlled = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetData(controlled);
		if (transient) {
			if (transient->IsInControl != nullptr) {
				return transient->IsInControl;
			}
		}
		return controlled;
	}



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 G T S   A C T O R   F U N C T I O N S                                                              //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float GetDamageSetting() {
		return Persistent::GetSingleton().size_related_damage_mult;
	}

	float GetFallModifier(Actor* giant) {
		auto transient = Transient::GetSingleton().GetData(giant);
		float fallmod = 1.0;
		if (transient) {
			fallmod = transient->FallTimer;
			log::info("Fall mult :{}", transient->FallTimer);
		}
		return fallmod;
	}
	
	float GetHPThreshold(Actor* actor) {
		float hp = 0.20;
		if (Runtime::HasPerkTeam(actor, "HugCrush_MightyCuddles")) {
			hp += 0.10; // 0.30
		}
		if (Runtime::HasPerkTeam(actor, "HugCrush_HugsOfDeath")) {
			hp += 0.20; // 0.50
		}
		return hp;
	}

	float Ench_Aspect_GetPower(Actor* giant) { 
		float aspect = SizeManager::GetSingleton().GetEnchantmentBonus(giant) * 0.01;
		return aspect;
	}
	float Ench_Hunger_GetPower(Actor* giant) {
		float hunger = SizeManager::GetSingleton().GetSizeHungerBonus(giant) * 0.01;
		return hunger;
	}

	float GetDamageResistance(Actor* actor) {
		return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kHealth);
	}

	float GetDamageMultiplier(Actor* actor) {
		return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kAttackDamageMult);
	}

	float Damage_CalculateSizeDamage(Actor* giant, Actor* tiny) {
		return GetDamageMultiplier(giant) * GetDamageResistance(tiny);
	}

	float GetSizeDifference(Actor* giant, Actor* tiny, bool Check_SMT, bool HH) {
		float hh_gts = 0.0; 
		float hh_tiny = 0.0;

		if (HH) { // Apply HH only in cases when we need it, such as damage and hugs
			hh_gts = HighHeelManager::GetHHOffset(giant)[2] * 0.01;
			hh_tiny = HighHeelManager::GetHHOffset(tiny)[2] * 0.01;
		}

		float GiantScale = (get_visual_scale(giant) + hh_gts) * GetSizeFromBoundingBox(giant);
		float TinyScale = (get_visual_scale(tiny) + hh_tiny) * GetSizeFromBoundingBox(tiny);

		if (Check_SMT) {
			if (HasSMT(giant)) {
				GiantScale += 9.8;
			} 
		}

		if (tiny->formID == 0x14 && HasSMT(tiny)) {
			TinyScale += 1.50;
		}

		float Difference = GiantScale/TinyScale;

		return Difference;
	}

	float GetActorWeight(Actor* giant, bool metric) {
		float hh = HighHeelManager::GetBaseHHOffset(giant)[2]/100;
		float scale = get_visual_scale(giant);
		float smt = 1.0;
		float totalscale = scale + (hh*0.10 * scale);
		float actorweight = 1.0 + giant->GetWeight()/300;
		float weight;
		if (metric) { // 70.1 kg as a base
			weight = 70.1 * actorweight * (totalscale * totalscale * totalscale);
		} else {
			weight = (70.1 * actorweight * (totalscale * totalscale * totalscale)) * 2.205;
		} if (HasSMT(giant)) {
			smt = 6.0;
		}
		return weight * smt;
	}

	float GetActorHeight(Actor* giant, bool metric) {
		float hh = HighHeelManager::GetBaseHHOffset(giant)[2]/100;
		float scale = get_visual_scale(giant);
		float smt = 1.0;
		float height;
		if (metric) { // 1.82 m as a base
			height = 1.82 * scale + (hh * scale); // meters
		} else {
			height = (1.82 * scale + (hh * scale)) * 3.28; // ft
		}
		return height;
	}

	float GetSizeFromBoundingBox(Actor* tiny) {
		auto profiler = Profilers::Profile("ActorUtils: GetSizeFromBoundingBox");
		float sc = get_bounding_box_to_mult(tiny);
		return sc;
	}

	float GetRoomStateScale(Actor* giant) {
		// Goal is to make us effectively smaller during these checks, so RayCast won't adjust our height unless we're truly too big
		float Normal = 1.0;
		float Reduction = 1.0;

		if (IsProning(giant)) {
			return 0.30;
		} else if (IsCrawling(giant)) {
			return 0.46;
		} else if (giant->IsSneaking()) {
			Reduction = 0.70;
		} else {
			Reduction = 1.0;
		}
		float HH = (HighHeelManager::GetBaseHHOffset(giant).Length()/100)/1.82; // Get HH value and convert it to meters
		return (Normal + HH) * Reduction;
	}

	float GetProneAdjustment() {
		auto player = PlayerCharacter::GetSingleton();
		float value = 1.0;
		if (IsProning(player)) {
			return 0.18;
		} else if (IsCrawling(player)) {
			value = std::clamp(Runtime::GetFloat("ProneOffsetFP"), 0.10f, 1.0f);
		}
		
		return value;
	}

	void SpawnActionIcon(Actor* giant) {
		if (!giant) {
			return;
		}
		bool enabled = Persistent::GetSingleton().EnableIcons;
		if (!enabled) {
			return;
		}
		static Timer EffectTimer = Timer(3.0);
		if (giant->formID == 0x14 && EffectTimer.ShouldRunFrame()) {
			NiPoint3 NodePosition = giant->GetPosition();

			float giantScale = get_visual_scale(giant);
			auto huggedActor = HugShrink::GetHuggiesActor(giant);

			const float BASE_DISTANCE = 124.0;
			float CheckDistance = BASE_DISTANCE * giantScale;

			if (IsCrawling(giant)) {
				CheckDistance *= 1.5;
			}

			if (IsDebugEnabled()) {
				DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 60, {0.5, 1.0, 0.0, 0.5});
			}

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (otherActor->Is3DLoaded() && !otherActor->IsDead()) {
						float tinyScale = get_visual_scale(otherActor) * GetSizeFromBoundingBox(otherActor);
						float difference = GetSizeDifference(giant, otherActor, true, false);
						if (difference > 5.8 || huggedActor) {
							NiPoint3 actorLocation = otherActor->GetPosition();
							if ((actorLocation - NodePosition).Length() < CheckDistance) {
								int nodeCollisions = 0;
								float force = 0.0;

								auto model = otherActor->GetCurrent3D();

								if (model) {
									VisitNodes(model, [&nodeCollisions, &force, NodePosition, CheckDistance](NiAVObject& a_obj) {
										float distance = (NodePosition - a_obj.world.translate).Length();
										if (distance < CheckDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / CheckDistance;
											return false;
										}
										return true;
									});
								}
								if (nodeCollisions > 0) {
									auto node = find_node(otherActor, "NPC Root [Root]");
									if (node) {
										auto grabbedActor = Grab::GetHeldActor(giant);
										float correction = 0; 
										if (tinyScale < 1.0) {
											correction = std::clamp((18.0f / tinyScale) - 18.0f, 0.0f, 144.0f);
										} else {
											correction = (18.0 * tinyScale) - 18.0;
										}

										float iconScale = std::clamp(tinyScale, 1.0f, 9999.0f) * 2.4;
										bool Ally = !IsHostile(giant, otherActor) && IsTeammate(otherActor);
										bool HasLovingEmbrace = Runtime::HasPerkTeam(giant, "HugCrush_LovingEmbrace");
										bool Healing = IsHugHealing(giant);

										NiPoint3 Position = node->world.translate;
										float bounding_z = get_bounding_box_z(otherActor);
										if (bounding_z > 0.0) {
											Position.z += (bounding_z * 2.5 * tinyScale); // 2.5 to be slightly above the head
											//log::info("For Actor: {}", otherActor->GetDisplayFullName());
											//log::info("---	Position: {}", Vector2Str(Position));
											//log::info("---	Actor Position: {}", Vector2Str(otherActor->GetPosition()));
											//log::info("---	Bounding Z: {}, Bounding Z * Scale: {}", bounding_z, bounding_z * tinyScale);
										} else {
											Position.z -= correction;
										}
										

										if (grabbedActor && grabbedActor == otherActor) {
											//do nothing
										} else if (huggedActor && huggedActor == otherActor && Ally && HasLovingEmbrace && !Healing) {
											SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_LovingEmbrace.nif", NiMatrix3(), Position, iconScale, 7, node);
										} else if (huggedActor && huggedActor == otherActor && !IsHugCrushing(giant) && !Healing) {
											bool LowHealth = (GetHealthPercentage(huggedActor) < GetHPThreshold(giant));
											bool ForceCrush = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles");
											float Stamina = GetStaminaPercentage(giant);
											if (HasSMT(giant) || LowHealth || (ForceCrush && Stamina > 0.50)) {
												SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Hug_Crush.nif", NiMatrix3(), Position, iconScale, 7, node); // Spawn 'can be hug crushed'
											}
										} else if (!IsGtsBusy(giant) && IsEssential(otherActor)) {
											SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Essential.nif", NiMatrix3(), Position, iconScale, 7, node); 
											// Spawn Essential icon
										} else if (!IsGtsBusy(giant) && difference >= Action_Crush) {
											if (CanPerformAnimation(giant, 3)) {
												SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Crush_All.nif", NiMatrix3(), Position, iconScale, 7, node); 
												// Spawn 'can be crushed and any action can be done'
											} else {
												SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Crush.nif", NiMatrix3(), Position, iconScale, 7, node); 
												// just spawn can be crushed, can happen at any quest stage
											}
										} else if (!IsGtsBusy(giant) && difference >= Action_Grab) {
											if (CanPerformAnimation(giant, 3)) {
												SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Vore_Grab.nif", NiMatrix3(), Position, iconScale, 7, node); 
												// Spawn 'Can be grabbed/vored'
											} else if (CanPerformAnimation(giant, 2)) {
												SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Grab.nif", NiMatrix3(), Position, iconScale, 7, node); 
												// Spawn 'Can be grabbed'
											}
										} else if (!IsGtsBusy(giant) && difference >= Action_Sandwich && CanPerformAnimation(giant, 2)) {
											SpawnParticle(otherActor, 3.00, "GTS/UI/Icon_Sandwich.nif", NiMatrix3(), Position, iconScale, 7, node); // Spawn 'Can be sandwiched'
										} 
										// 1 = stomps and kicks
										// 2 = Grab and Sandwich
										// 3 = Vore
										// 5 = Others
									}
								}
							}
						}
					}
				}
			}
		}
	}
	

	void update_target_scale(Actor* giant, float amt, SizeEffectType type) { // used to mod scale with perk bonuses taken into account
		float OTE = 1.0;
		float scale = get_visual_scale(giant);
		bool perk = Runtime::HasPerkTeam(giant, "OnTheEdge");
		if (amt > 0 && (giant->formID == 0x14 || IsTeammate(giant))) {
			if (scale >= 1.0) {
				amt /= GetGrowthReduction(scale);
				// Enabled if BalanceMode is True. Decreases Grow Efficiency.
			}
		} else if (amt - EPS < 0.0) {
			// If neative change: add stolen attributes
			DistributeStolenAttributes(giant, -amt * GetGrowthReduction(scale)); // Adjust max attributes
		}
		if (type == SizeEffectType::kShrink) {
			OTE = GetPerkBonus_OnTheEdge(giant, amt);
		}

		mod_target_scale(giant, amt * OTE); // set target scale value
	}




	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 G T S   S T A T E S  S E T S                                                                       //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetBeingHeld(Actor* tiny, bool decide) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->being_held = decide;
		}
	}
	void SetProneState(Actor* giant, bool enable) {
		if (giant->formID == 0x14) {
			auto transient = Transient::GetSingleton().GetData(giant);
			if (transient) {
				transient->FPProning = enable;
			}
		}
	}
	void SetBetweenBreasts(Actor* actor, bool decide) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->between_breasts = decide;
		}
	}
	void SetBeingEaten(Actor* tiny, bool decide) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->about_to_be_eaten = decide;
		}
	}
	void SetBeingGrinded(Actor* tiny, bool decide) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->being_foot_grinded = decide;
		}
	}

	void SetCameraOverride(Actor* actor, bool decide) {
		if (actor->formID == 0x14) {
			auto transient = Transient::GetSingleton().GetData(actor);
			if (transient) {
				transient->OverrideCamera = decide;
			}
		}
	}

	void SetReanimatedState(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (!WasReanimated(actor)) { // disallow to override it again if it returned true a frame before
			bool reanimated = actor->AsActorState()->GetLifeState() == ACTOR_LIFE_STATE::kReanimate;
			if (transient) {
				transient->WasReanimated = reanimated;
				//Cprint("Set {} to reanimated: {}", actor->GetDisplayFullName(), reanimated);
			}
		}
	}

	void ShutUp(Actor* actor) { // Disallow them to "So anyway i've been fishing today and my dog died" while we do something to them
		if (!actor) {
			return;
		}
		if (actor->formID != 0x14) {
			auto ai = actor->GetActorRuntimeData().currentProcess;
			if (ai) {
				if (ai->high) {
					float Greeting = ai->high->greetingTimer;
					ai->high->greetingTimer = 5;
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PlayAnimation(Actor* actor, std::string_view animName) {
		actor->NotifyAnimationGraph(animName);
	}

	void Disintegrate(Actor* actor, bool script) {
		std::string taskname = std::format("Disintegrate_{}", actor->formID);
		auto tinyref = actor->CreateRefHandle();
		TaskManager::RunOnce(taskname, [=](auto& update) {
			if (!tinyref) {
				return;
			}
			auto tiny = tinyref.get().get();

			bool dragon = IsDragon(tiny);
			if (dragon) {
				tiny->Disable();
				return;
			}

			SetCriticalStage(tiny, 4);
			tiny->Disable();
		});
	}



	void UnDisintegrate(Actor* actor) {
		//actor->GetActorRuntimeData().criticalStage.reset(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
	}

	void SetRestrained(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetRestrained", true);
	}

	void SetUnRestrained(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetRestrained", false);
	}

	void SetDontMove(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetDontMove", true);
	}

	void SetMove(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetDontMove", true);
	}

	void ForceRagdoll(Actor* actor, bool forceOn) {
		if (!actor) {
			return;
		}
		auto charCont = actor->GetCharController();
		if (!charCont) {
			return;
		}
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					if (graph->HasRagdoll()) {
						if (forceOn) {
							graph->AddRagdollToWorld();
							charCont->flags.set(CHARACTER_FLAGS::kFollowRagdoll);
						} else {
							graph->RemoveRagdollFromWorld();
							charCont->flags.reset(CHARACTER_FLAGS::kFollowRagdoll);
						}
					}
				}
			}
		}
	}


	std::vector<hkpRigidBody*> GetActorRB(Actor* actor) {
		std::vector<hkpRigidBody*> results = {};
		auto charCont = actor->GetCharController();
		if (!charCont) {
			return results;
		}

		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charCont);
		bhkCharRigidBodyController* charRigidBodyController = skyrim_cast<bhkCharRigidBodyController*>(charCont);
		if (charProxyController) {
			// Player controller is a proxy one
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);

				if (hkpObject) {
					// Not sure what bodies is doing
					for (auto body: hkpObject->bodies) {
						results.push_back(body);
					}
					// // This one appears to be active during combat.
					// // Maybe used for sword swing collision detection
					// for (auto phantom: hkpObject->phantoms) {
					// 	results.push_back(phantom);
					// }
					//
					// // This is the actual shape
					// if (hkpObject->shapePhantom) {
					// 	results.push_back(hkpObject->shapePhantom);
					// }
				}
			}
		} else if (charRigidBodyController) {
			// NPCs seem to use rigid body ones
			auto& characterRigidBody = charRigidBodyController->characterRigidBody;
			hkReferencedObject* refObject = characterRigidBody.referencedObject.get();
			if (refObject) {
				hkpCharacterRigidBody* hkpObject = skyrim_cast<hkpCharacterRigidBody*>(refObject);
				if (hkpObject) {
					if (hkpObject->m_character) {
						results.push_back(hkpObject->m_character);
					}
				}
			}
		}

		return results;
	}

	void PushActorAway(TESObjectREFR* source, Actor* receiver, float afKnockBackForce) {
		if (receiver->IsDead() || IsRagdolled(receiver)) {
			return;
		}

		if (source) {
			auto ai = receiver->GetActorRuntimeData().currentProcess;
			if (ai) {
				if (ai->InHighProcess()) {
					if (receiver->Is3DLoaded()) {
						if (source->Is3DLoaded()) {
							NiPoint3 direction = receiver->GetPosition() - source->GetPosition();
							direction = direction / direction.Length();

							typedef void (*DefPushActorAway)(AIProcess *ai, Actor* actor, NiPoint3& direction, float force);
							REL::Relocation<DefPushActorAway> RealPushActorAway{ RELOCATION_ID(38858, 39895) };
							RealPushActorAway(ai, receiver, direction, afKnockBackForce);
						}
					}
				}
			}
		}
	}

	void PushActorAway(TESObjectREFR* source, Actor* receiver, NiPoint3 direction, float force) {
		if (receiver->IsDead() || IsRagdolled(receiver)) {
			return;
		}

		if (source) {
			auto ai = receiver->GetActorRuntimeData().currentProcess;
			if (ai) {
				if (ai->InHighProcess()) {
					if (receiver->Is3DLoaded()) {
						if (source->Is3DLoaded()) {
							typedef void (*DefPushActorAway)(AIProcess *ai, Actor* actor, NiPoint3& direction, float force);
							REL::Relocation<DefPushActorAway> RealPushActorAway{ RELOCATION_ID(38858, 39895) };
							RealPushActorAway(ai, receiver, direction, force);
						}
					}
				}
			}
		}
	}

	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius) {
		CallFunctionOn(source, "ObjectReference", "KnockAreaEffect", afMagnitude, afRadius);
	}
	void ApplyHavokImpulse_Manual(Actor* target, float afX, float afY, float afZ, float afMagnitude) {
		NiPoint3 direction = NiPoint3(afX, afY, afZ);
		//NiPoint3 niImpulse = direction * afMagnitude/direction.Length();
		//hkVector4 impulse = hkVector4(niImpulse.x, niImpulse.y, niImpulse.z, 0.0);
		hkVector4 impulse = hkVector4(afX, afY, afZ, afMagnitude);
		auto rbs = GetActorRB(target);
		for (auto rb: rbs) {
			if (rb) {
				auto& motion = rb->motion;
				motion.ApplyLinearImpulse(impulse);
			}
		}
	}
	void ApplyHavokImpulse(TESObjectREFR* target, float afX, float afY, float afZ, float afMagnitude) {
		CallFunctionOn(target, "ObjectReference", "ApplyHavokImpulse", afX, afY, afZ, afMagnitude);
	}

	void CompleteDragonQuest(Actor* tiny, bool vore, bool dead) {
		auto pc = PlayerCharacter::GetSingleton();
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			auto stage = progressionQuest->GetCurrentStageID();
			if (stage == 80) {
				auto transient = Transient::GetSingleton().GetData(pc);
				if (transient) {
					Cprint("Quest is Completed");
					transient->dragon_was_eaten = true;
					SpawnProgressionParticle(tiny, vore);
				}
			}
		}
	}

	float GetHighHeelsBonusDamage(Actor* actor) {
		if (!actor) {
			return false;
		}
		auto profiler = Profilers::Profile("ActorUtils: GetHHBonusDamage");
		if (Runtime::HasPerkTeam(actor, "hhBonus")) {
			return HighHeelManager::GetBaseHHOffset(actor).Length()/100;
		}
		return 0.0;
	}

	float get_distance_to_actor(Actor* receiver, Actor* target) {
		auto profiler = Profilers::Profile("ActorUtils: GetDistanceToActor");
		if (target) {
			auto point_a = receiver->GetPosition();
			auto point_b = target->GetPosition();
			auto delta = point_a - point_b;
			return delta.Length();
		}
		return 3.4028237E38; // Max float
	}

	void ApplyShake(Actor* caster, float modifier, float radius) {
		if (caster) {
			auto position = caster->GetPosition();
			ApplyShakeAtPoint(caster, modifier, position, radius);
		}
	}

	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view nodesv) {
		auto node = find_node(caster, nodesv);
		if (node) {
			ApplyShakeAtPoint(caster, modifier, node->world.translate, 1.0);
		}
	}

	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view nodesv, float radius) {
		auto node = find_node(caster, nodesv);
		if (node) {
			ApplyShakeAtPoint(caster, modifier, node->world.translate, radius);
		}
	}

	void ApplyShakeAtPoint(Actor* caster, float modifier, const NiPoint3& coords, float radius) {
		if (!caster) {
			return;
		}
		// Reciever is always PC if it is not PC we do nothing anyways
		Actor* receiver = PlayerCharacter::GetSingleton();
		if (!receiver) {
			return;
		}

		float distance = get_distance_to_camera(coords);
		float sourcesize = get_visual_scale(caster);
		float receiversize = get_visual_scale(receiver);
		float sizedifference = (sourcesize/receiversize);
		if (caster->formID == 0x14) {
			sizedifference = sourcesize;
		}


		// To Sermit: You wrote a cutoff not a falloff
		//            was this intentional?
		//
		// FYI: This is the difference
		// Falloff:
		//   |
		// I |----\
		//   |     \
		//   |______\___
		//        distance
		// Cuttoff:
		//   |
		// I |----|
		//   |    |
		//   |____|_____
		//        distance
		float cuttoff = 450 * sizedifference * radius;
		if (distance < cuttoff) {
			// To Sermit: Same value as before just with the math reduced to minimal steps
			float intensity = (sizedifference * 18.8) / distance;
			float duration = 0.25 * intensity * (1 + (sizedifference * 0.25));
			intensity = std::clamp(intensity, 0.0f, 1e8f);
			duration = std::clamp(duration, 0.0f, 1.2f);


			shake_controller(intensity*modifier, intensity*modifier, duration);
			shake_camera_at_node(coords, intensity*modifier, duration);
		}
	}

	void EnableFreeCamera() {
		auto playerCamera = PlayerCamera::GetSingleton();
		playerCamera->ToggleFreeCameraMode(false);
	}

	bool DisallowSizeDamage(Actor* giant, Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			if (transient->Protection == false) {
				return false;
			} 

			bool Hostile = IsHostile(giant, tiny);
			return transient->Protection && !Hostile;
		}
		
		return false;
	}

	bool AllowDevourment() {
		return Persistent::GetSingleton().devourment_compatibility;
	}

	bool AllowCameraTracking() {
		return Persistent::GetSingleton().allow_feetracking;
	}
	bool LessGore() {
		return Persistent::GetSingleton().less_gore;
	}

	bool IsTeammate(Actor* actor) {
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) { 
			return true;
		}
		return false;
	}

	bool EffectsForEveryone(Actor* giant) { // determines if we want to apply size effects for literally every single actor
		if (giant->formID == 0x14) { // don't enable for Player
			return false;
		}
		float scale = get_visual_scale(giant);
		if (scale > 1.10) {
			bool dead = giant->IsDead();
			bool everyone = Runtime::GetBool("PreciseDamageOthers");
			if (!dead && everyone) {
				return true;
			} else {
				return false;
			}
		}
		return false;
	}

	void ResetCameraTracking() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto& sizemanager = SizeManager::GetSingleton();
			sizemanager.SetTrackedBone(player, false, CameraTracking::None);
		}
	}

	void CallDevourment(Actor* giant, Actor* tiny) {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "Devourment", giant, tiny);
		}
	}

	void CallGainWeight(Actor* giant, float value) {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "GainWeight", giant, value);
		}
	}

	void CallVampire() {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "SatisfyVampire");
		}
	}

	void CallHelpMessage() {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "TrueGiantessMessage");
		}
	}

	void AddCalamityPerk() {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "AddCalamityPerk");
		}
	}

	void AddPerkPoints(float level) {
		auto GtsSkillPerkPoints = Runtime::GetGlobal("GtsSkillPerkPoints");
		if (!GtsSkillPerkPoints) {
			return;
		}
		if (int(level) % 5 == 0) {
			Notify("You've learned a bonus perk point");
			GtsSkillPerkPoints->value += 1.0;
		}
		if (level == 20 || level == 40) {
			GtsSkillPerkPoints->value += 2.0;
		} else if (level == 60 || level == 80) {
			GtsSkillPerkPoints->value += 3.0;
		} else if (level == 100) {
			GtsSkillPerkPoints->value += 4.0;
		}
	}

	void AddStolenAttributes(Actor* giant, float value) {
		if (giant->formID == 0x14 && Runtime::HasPerk(giant, "SizeAbsorption")) {
			auto attributes = Persistent::GetSingleton().GetData(giant);
			if (attributes) {
				//log::info("Adding {} to stolen attributes", value);
				attributes->stolen_attributes += value;

				if (attributes->stolen_attributes <= 0.0) {
					attributes->stolen_attributes = 0.0; // Cap it just in case
				}
				//log::info("Stolen AV value: {}", attributes->stolen_attributes);
			}
		}
	}

	void AddStolenAttributesTowards(Actor* giant, ActorValue type, float value) {
		if (giant->formID == 0x14) {
			auto Persistent = Persistent::GetSingleton().GetData(giant);
			if (Persistent) {
				float modifier = Runtime::GetFloatOr("LevelLimitModifier", 1.0);
				float& health = Persistent->stolen_health;
				float& magick = Persistent->stolen_magick;
				float& stamin = Persistent->stolen_stamin;
				float limit = 2.0 * giant->GetLevel() * modifier;
				if (type == ActorValue::kHealth) {
					health += value;
					if (health >= limit) {
						health = limit;
					}
					//log::info("Adding {} to health, health: {}", value, health);
				} else if (type == ActorValue::kMagicka) {
					magick += value;
					if (magick >= limit) {
						magick = limit;
					}
					//log::info("Adding {} to magick, magicka: {}", value, magick);
				} else if (type == ActorValue::kStamina) {
					stamin += value;
					if (stamin >= limit) {
						stamin = limit;
					}
					//log::info("Adding {} to stamina, stamina: {}", value, stamin);
				}
			}
		}
	}

	float GetStolenAttributes_Values(Actor* giant, ActorValue type) {
		if (giant->formID == 0x14) {
			auto Persistent = Persistent::GetSingleton().GetData(giant);
			if (Persistent) {
				if (type == ActorValue::kHealth) {
					return Persistent->stolen_health;
				} else if (type == ActorValue::kMagicka) {
					return Persistent->stolen_magick;
				} else if (type == ActorValue::kStamina) {
					return Persistent->stolen_stamin;
				} else {
					return 0.0;
				}
			}
			return 0.0;
		}
		return 0.0;
	}

	float GetStolenAttributes(Actor* giant) {
		auto persist = Persistent::GetSingleton().GetData(giant);
		if (persist) {
			return persist->stolen_attributes;
		}
		return 0.0;
	}

	void DistributeStolenAttributes(Actor* giant, float value) {
		if (value > 0 && giant->formID == 0x14 && Runtime::HasPerk(giant, "SizeAbsorption")) { // Permamently increases random AV after shrinking and stuff
			float scale = std::clamp(get_visual_scale(giant), 0.01f, 999999.0f);
			float modifier = Runtime::GetFloatOr("LevelLimitModifier", 1.0);
			float Storage = GetStolenAttributes(giant);
			float limit = 2.0 * giant->GetLevel() * modifier;


			auto Persistent = Persistent::GetSingleton().GetData(giant);
			if (!Persistent) {
				return;
			}
			//log::info("Adding {} to attributes", value);
			float& health = Persistent->stolen_health;
			float& magick = Persistent->stolen_magick;
			float& stamin = Persistent->stolen_stamin;

			if (Storage > 0.0) {
				int Boost = rand() % 3;
				if (Boost == 0) {
					health += (value * 4);
					if (health >= limit) {
						health = limit;
					}
					//log::info("Adding {} to HP, HP {}", value * 4, health);
				} else if (Boost == 1) {
					magick += (value * 4);
					if (magick >= limit) {
						magick = limit;
					}
					//log::info("Adding {} to MP, MP {}", value * 4, magick);
				} else if (Boost >= 2) {
					stamin += (value * 4);
					if (stamin >= limit) {
						stamin = limit;
					}
					//log::info("Adding {} to SP, SP {}", value * 4, stamin);
				}
				AddStolenAttributes(giant, -value); // reduce it
			}
		}
	}

	float GetRandomBoost() {
		float rng = (rand()% 150 + 1);
		float random = rng/100;
		return random;
	}

	float GetButtCrushCost(Actor* actor) {
		float cost = 1.0;
		if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
			cost -= 0.15;
		}
		if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
			cost -= 0.25;
		}
		cost *= Perk_GetCostReduction(actor);
		if (IsCrawling(actor)) {
			cost *= 1.35;
		}
		return cost;
	}

	float Perk_GetCostReduction(Actor* giant) {
		float cost = 1.0;
		float reduction_1 = 0.0;
		float reduction_2 = 1.0;
		if (Runtime::HasPerkTeam(giant, "SkilledGTS")) {
			reduction_1 += std::clamp(GetGtsSkillLevel() * 0.0035f, 0.0f, 0.35f);
		}
		if (giant->formID == 0x14 && HasGrowthSpurt(giant)) {
			if (Runtime::HasPerkTeam(giant, "GrowthOfStrength")) {
				reduction_2 -= 0.10;
			} 
			if (Runtime::HasPerk(giant, "ExtraGrowth")) {
				reduction_2 -= 0.30;
			}
		}
		cost -= reduction_1;
		cost *= reduction_2;
		return cost;
	}

	float GetAnimationSlowdown(Actor* giant) {
		if (!giant) {
			return 1.0;
		}
		float scale = get_visual_scale(giant);
		SoftPotential getspeed {
			.k = 0.142, // 0.125
			.n = 0.82, // 0.86
			.s = 1.90, // 1.12
			.o = 1.0,
			.a = 0.0,  //Default is 0
		};
		float speedmultcalc = soft_core(scale, getspeed);
		return speedmultcalc;
	}

	void DoFootstepSound(Actor* giant, float modifier, FootEvent kind, std::string_view node) {
		auto& footstep = FootStepManager::GetSingleton();
		Impact impact_data = Impact {
			.actor = giant,
			.kind = kind,
			.scale = get_visual_scale(giant) * modifier,
			.nodes = find_node(giant, node),
		};
		footstep.OnImpact(impact_data); // Play sound
	}

	void DoDustExplosion(Actor* giant, float modifier, FootEvent kind, std::string_view node) {
		auto& explosion = ExplosionManager::GetSingleton();
		Impact impact_data = Impact {
			.actor = giant,
			.kind = kind,
			.scale = get_visual_scale(giant) * modifier,
			.nodes = find_node(giant, node),
		};
		explosion.OnImpact(impact_data); // Play explosion
	}

	void SpawnParticle(Actor* actor, float lifetime, const char* modelName, const NiMatrix3& rotation, const NiPoint3& position, float scale, std::uint32_t flags, NiAVObject* target) {
		auto cell = actor->GetParentCell();
		if (cell) {
			BSTempEffectParticle::Spawn(cell, lifetime, modelName, rotation, position, scale, flags, target);
		}
	}

	void SpawnDustParticle(Actor* giant, Actor* tiny, std::string_view node, float size) {
		auto result = find_node(giant, node);
		if (result) {
			BGSExplosion* base_explosion = Runtime::GetExplosion("draugrexplosion");
			if (base_explosion) {
				NiPointer<TESObjectREFR> instance_ptr = giant->PlaceObjectAtMe(base_explosion, false);
				if (!instance_ptr) {
					return;
				}
				TESObjectREFR* instance = instance_ptr.get();
				if (!instance) {
					return;
				}
				Explosion* explosion = instance->AsExplosion();
				if (!explosion) {
					return;
				}
				explosion->SetPosition(result->world.translate);
				explosion->GetExplosionRuntimeData().radius *= 3 * get_visual_scale(tiny) * size;
				explosion->GetExplosionRuntimeData().imodRadius *= 3 * get_visual_scale(tiny) * size;
				explosion->GetExplosionRuntimeData().unkB8 = nullptr;
				explosion->GetExplosionRuntimeData().negativeVelocity *= 0.0;
				explosion->GetExplosionRuntimeData().unk11C *= 0.0;
			}
		}
	}

	void StaggerOr(Actor* giant, Actor* tiny, float afX, float afY, float afZ, float afMagnitude) {
		if (tiny->IsDead()) {
			return;
		}
		if (InBleedout(tiny)) {
			return;
		}
		if (IsBeingHeld(tiny)) {
			return;
		}
		if (!AllowStagger(giant, tiny)) {
			return;
		} 

		float giantSize = get_visual_scale(giant);
		float tinySize = get_visual_scale(tiny); 

		if (HasSMT(giant)) {
			giantSize += 1.0;
		} if (tiny->formID == 0x14 && HasSMT(tiny)) {
			tinySize += 1.25;
		}

		float sizedifference = giantSize/tinySize;
		float sizedifference_tinypov = tinySize/giantSize;

		int ragdollchance = rand() % 30 + 1.0;
		if (!IsRagdolled(tiny) && sizedifference > 2.8 && ragdollchance < 4.0 * sizedifference) { // Chance for ragdoll. Becomes 100% at high scales
			PushActorAway(giant, tiny, 1.0); // Ragdoll
			return;
		} else if (sizedifference > 1.25) { // Always Stagger
			tiny->SetGraphVariableFloat("GiantessScale", sizedifference_tinypov); // enable stagger just in case

		    float push = std::clamp(0.25f * (sizedifference - 0.25f), 0.25f, 1.0f);
			StaggerActor(giant, tiny, push);
			return;
		}
	}

	void Utils_PushCheck(Actor* giant, Actor* tiny, float force) {
		auto model = tiny->GetCurrent3D();
		auto& sizemanager = SizeManager::GetSingleton();
	
		bool moving = IsMoving(giant);

		if (model) {
			bool isdamaging = IsActionOnCooldown(tiny, CooldownSource::Push_Basic);
			if (!isdamaging && (force >= 0.12 || moving || IsFootGrinding(giant))) {
				StaggerOr(giant, tiny, 0, 0, 0, 0);
				ApplyActionCooldown(tiny, CooldownSource::Push_Basic);
			}
		}
	}

	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage, FootEvent kind, float crushmult, DamageSource Cause) {
		//radius = 1.0;// + (GetHighHeelsBonusDamage(giant) * 2.5);
		if (kind == FootEvent::Left) {
			CollisionDamage::GetSingleton().DoFootCollision(giant, damage, radius, random, bonedamage, crushmult, Cause, false, false);
		}
		if (kind == FootEvent::Right) {
			CollisionDamage::GetSingleton().DoFootCollision(giant, damage, radius, random, bonedamage, crushmult, Cause, true, false);
			//                                                                                  ^        ^           ^ - - - - Normal Crush
			//                                                       Chance to trigger bone crush   Damage of            Threshold multiplication
			//                                                                                      Bone Crush
		}
	}

	void PushTowards(Actor* giantref, Actor* tinyref, std::string_view bone, float power, bool sizecheck) {
		NiAVObject* node = find_node(giantref, bone);
		if (node) {
			PushTowards(giantref, tinyref, node, power, sizecheck);
		}
	}

	void PushTowards(Actor* giantref, Actor* tinyref, NiAVObject* bone, float power, bool sizecheck) {
		NiPoint3 startCoords = bone->world.translate;
		double startTime = Time::WorldTimeElapsed();
		ActorHandle tinyHandle = tinyref->CreateRefHandle();
		ActorHandle gianthandle = giantref->CreateRefHandle();
		PushActorAway(giantref, tinyref, 1);
		// Do this next frame (or rather until some world time has elapsed)
		TaskManager::Run([=](auto& update){
			if (!gianthandle) {
				return false;
			}
			if (!tinyHandle) {
				return false;
			}
			Actor* giant = gianthandle.get().get();
			Actor* tiny = tinyHandle.get().get();
			

			NiPoint3 endCoords = bone->world.translate;
			double endTime = Time::WorldTimeElapsed();


			if ((endTime - startTime) > 1e-4) {
				// Time has elapsed

				NiPoint3 vector = endCoords - startCoords;
				float distanceTravelled = vector.Length();
				float timeTaken = endTime - startTime;
				float speed = distanceTravelled / timeTaken;
				NiPoint3 direction = vector / vector.Length();
				if (sizecheck) {
					float giantscale = get_visual_scale(giant);
					float tinyscale = get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny);

					if (tiny->IsDead()) {
						tinyscale *= 0.4;
					}

					if (HasSMT(giant)) {
						giantscale *= 6.0;
					}
					float sizedifference = giantscale/tinyscale;

					if (sizedifference < 1.2) {
						return false; // terminate task
					} else if (sizedifference > 1.2 && sizedifference < 3.0) {
						tiny->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
						tiny->NotifyAnimationGraph("staggerStart");
						return false; //Only Stagger
					}
				}
				// If we pass checks, launch actor instead
				TESObjectREFR* tiny_is_object = skyrim_cast<TESObjectREFR*>(tiny);
				if (tiny_is_object) {
					ApplyHavokImpulse(tiny_is_object, direction.x, direction.y, direction.z, speed * 2.0 * power);
				}
				return false;
			} else {
				return true;
			}
		});
	}

	void PushForward(Actor* giantref, Actor* tinyref, float power) {
		double startTime = Time::WorldTimeElapsed();
		ActorHandle tinyHandle = tinyref->CreateRefHandle();
		ActorHandle gianthandle = giantref->CreateRefHandle();
		std::string taskname = std::format("PushOther {}", tinyref->formID);
		PushActorAway(giantref, tinyref, 1);
		TaskManager::Run(taskname, [=](auto& update) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyHandle) {
				return false;
			}
			Actor* giant = gianthandle.get().get();
			Actor* tiny = tinyHandle.get().get();
			

			auto playerRotation = giant->GetCurrent3D()->world.rotate;
			RE::NiPoint3 localForwardVector{ 0.f, 1.f, 0.f };
			RE::NiPoint3 globalForwardVector = playerRotation * localForwardVector;

			RE::NiPoint3 direction = globalForwardVector;
			double endTime = Time::WorldTimeElapsed();

			if ((endTime - startTime) > 0.08) {
				// Time has elapsed
				TESObjectREFR* tiny_as_object = skyrim_cast<TESObjectREFR*>(tiny);
				if (tiny_as_object) {
					ApplyHavokImpulse(tiny_as_object, direction.x, direction.y, direction.z, power);
				}
				return false;
			} else {
				return true;
			}
		});
	}

	void TinyCalamityExplosion(Actor* giant, float radius) { // Meant to just stagger actors
		if (!giant) {
			return;
		}
		auto node = find_node(giant, "NPC Root [Root]");
		if (!node) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		NiPoint3 NodePosition = node->world.translate;
		const float maxDistance = radius;
		float totaldistance = maxDistance * giantScale;
		// Make a list of points to check
		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
			DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), totaldistance, 600, {0.0, 1.0, 0.0, 1.0});
		}

		NiPoint3 giantLocation = giant->GetPosition();

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				NiPoint3 actorLocation = otherActor->GetPosition();
				if ((actorLocation-giantLocation).Length() < (maxDistance*giantScale * 3.0)) {
					int nodeCollisions = 0;
					float force = 0.0;
					auto model = otherActor->GetCurrent3D();
					if (model) {
						VisitNodes(model, [&nodeCollisions, &force, NodePosition, totaldistance](NiAVObject& a_obj) {
							float distance = (NodePosition - a_obj.world.translate).Length();
							if (distance < totaldistance) {
								nodeCollisions += 1;
								force = 1.0 - distance / totaldistance;
								return false;
							}
							return true;
						});
					}
					if (nodeCollisions > 0) {
						float sizedifference = giantScale/get_visual_scale(otherActor);
						if (sizedifference <= 1.6) {
							StaggerActor(giant, otherActor, 0.75f);
						} else {
							PushActorAway(giant, otherActor, 1.0 * GetLaunchPower(giant, sizedifference));
						}
					}
				}
			}
		}
	}

	void ShrinkOutburst_Shrink(Actor* giant, Actor* tiny, float shrink, float gigantism) {
		if (IsEssential(tiny)) { // Protect followers/essentials
			return;
		}
		bool DarkArts1 = Runtime::HasPerk(giant, "DarkArts_Aug");
		bool DarkArts2 = Runtime::HasPerk(giant, "DarkArts_Aug2");

		float shrinkpower = (shrink * 0.35) * (1.0 + (GetGtsSkillLevel() * 0.005)) * CalcEffeciency(giant, tiny);

		float Adjustment = GetSizeFromBoundingBox(tiny);

		float sizedifference = GetSizeDifference(giant, tiny, true, false);
		if (DarkArts1) {
			giant->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, 8.0);
		}
		if (DarkArts2 && (IsGrowthSpurtActive(giant) || HasSMT(giant))) {
			shrinkpower *= 1.40;
		}

		update_target_scale(tiny, -(shrinkpower * gigantism), SizeEffectType::kShrink);
		Attacked(tiny, giant);

		ModSizeExperience(giant, (shrinkpower * gigantism) * 0.80);

		float MinScale = 0.11;

		if (get_target_scale(tiny) <= MinScale) {
			set_target_scale(tiny, MinScale);
		}
		if (sizedifference <= 4.0) { // Stagger or Push
			StaggerActor(giant, tiny, 0.25f);
		} else {
			PushActorAway(giant, tiny, 1.0/Adjustment * GetLaunchPower(giant, sizedifference));
		}
	}

	void ShrinkOutburstExplosion(Actor* giant, bool WasHit) {
		if (!giant) {
			return;
		}
		auto node = find_node(giant, "NPC Pelvis [Pelv]");
		if (!node) {
			return;
		}
		NiPoint3 NodePosition = node->world.translate;

		float giantScale = get_visual_scale(giant);
		float gigantism = 1.0 + Ench_Aspect_GetPower(giant);
		float shrink = 0.38;
		float radius = 1.0;

		float explosion = 0.75;
		bool DarkArts1 = Runtime::HasPerk(giant, "DarkArts_Aug");
		if (WasHit) {
			radius *= 1.4;
			shrink += 0.20;
			explosion += 0.95;
		}
		if (DarkArts1) {
			radius *= 1.33;
			shrink *= 1.33;
			explosion += 0.30;
		}

		const float BASE_DISTANCE = 84.0;
		float CheckDistance = BASE_DISTANCE*giantScale*gigantism*radius;

		Runtime::PlaySoundAtNode("ShrinkOutburstSound", giant, explosion, 1.0, "NPC Pelvis [Pelv]");
		GRumble::For("ShrinkOutburst", giant, 20.0, 0.15, "NPC COM [COM ]", 0.60);

		SpawnParticle(giant, 6.00, "GTS/Shouts/ShrinkOutburst.nif", NiMatrix3(), NodePosition, giantScale*explosion*3.0, 7, nullptr); // Spawn effect

		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
			DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 600, {0.0, 1.0, 0.0, 1.0});
		}

		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				NiPoint3 actorLocation = otherActor->GetPosition();
				if ((actorLocation - giantLocation).Length() < BASE_DISTANCE*giantScale*radius*3) {
					int nodeCollisions = 0;
					float force = 0.0;

					auto model = otherActor->GetCurrent3D();

					if (model) {
						VisitNodes(model, [&nodeCollisions, &force, NodePosition, CheckDistance](NiAVObject& a_obj) {
							float distance = (NodePosition - a_obj.world.translate).Length();
							if (distance < CheckDistance) {
								nodeCollisions += 1;
								force = 1.0 - distance / CheckDistance;
								return false;
							}
							return true;
						});
					}
					if (nodeCollisions > 0) {
						ShrinkOutburst_Shrink(giant, otherActor, shrink, gigantism);
					}
				}
			}
		}
	}

	void Utils_ProtectTinies(bool Balance) { // This is used to avoid damaging friendly actors in towns and in general
		auto player = PlayerCharacter::GetSingleton();

		//to-do: make it consume health over time in balance mode
		// And make it self-dispel if health is < threshold + play audio effect and push actors away, as well as stagger gts

		for (auto actor: find_actors()) {
			if (actor == player || IsTeammate(actor)) {
				float scale = get_visual_scale(actor);
				auto node = find_node(actor, "NPC Root [Root]");
				if (node) {
					NiPoint3 pos = node->world.translate;
					SpawnParticle(actor, 4.60, "GTS/Magic/Life_Drain.nif", NiMatrix3(), pos, scale * 1.15, 7, nullptr);
					Runtime::PlaySoundAtNode("Magic_ProtectTinies", actor, 1.0, 1.0, "NPC COM [COM ]");

					std::string name_com = std::format("Protect_{}", actor->formID);
					std::string name_root = std::format("Protect_Root_{}", actor->formID);

					GRumble::Once(name_com, actor, 8.6, 0.20, "NPC COM [COM ]");
					GRumble::Once(name_root, actor, 8.6, 0.20, "NPC Root [Root]");
				}

				LaunchImmunityTask(actor, Balance);
			}
		}
	}

	void LaunchImmunityTask(Actor* giant, bool Balance) {
		auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			transient->Protection = true;
		}

		std::string name = std::format("Protect_{}", giant->formID);

		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = giant->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}

			float Finish = Time::WorldTimeElapsed();
			float timepassed = Finish - Start;
			if (timepassed < 60.0) {
				auto giantref = gianthandle.get().get();
				if (Utils_ManageTinyProtection(giant, false, Balance)) {
					return true; // Disallow to check further
				}
			}
			if (transient) {
				transient->Protection = false; // reset protection to default value
			}
			return Utils_ManageTinyProtection(giant, true, Balance); // stop task, immunity has ended
		});
	}


	bool HasSMT(Actor* giant) {
		if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat")) {
			return true;
		}
		return false;
	}

	void TiredSound(Actor* player, std::string_view message) {
		if (player->formID != 0x14) {
			return;
		}
		if (IsFirstPerson()) {
			return;
		}
		static Timer Cooldown = Timer(1.2);
		if (Cooldown.ShouldRun()) {
			Runtime::PlaySound("VoreSound_Fail", player, 0.7, 1.0);
			Notify(message);
		}
	}

	hkaRagdollInstance* GetRagdoll(Actor* actor) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							return ragdoll;
						}
					}
				}
			}
		}
		return nullptr;
	}

	void ManageRagdoll(Actor* tiny, float deltaLength, NiPoint3 deltaLocation, NiPoint3 targetLocation) {
		if (deltaLength >= 70.0) {
			// WARP if > 1m
			auto ragDoll = GetRagdoll(tiny);
			hkVector4 delta = hkVector4(deltaLocation.x/70.0, deltaLocation.y/70.0, deltaLocation.z/70, 1.0);
			for (auto rb: ragDoll->rigidBodies) {
				if (rb) {
					auto ms = rb->GetMotionState();
					if (ms) {
						hkVector4 currentPos = ms->transform.translation;
						hkVector4 newPos = currentPos + delta;
						rb->motion.SetPosition(newPos);
						rb->motion.SetLinearVelocity(hkVector4(0.0, 0.0, -10.0, 0.0));
					}
				}
			}
		} else {
			// Just move the hand if <1m
			std::string_view handNodeName = "NPC HAND L [L Hand]";
			auto handBone = find_node(tiny, handNodeName);
			if (handBone) {
				auto collisionHand = handBone->GetCollisionObject();
				if (collisionHand) {
					auto handRbBhk = collisionHand->GetRigidBody();
					if (handRbBhk) {
						auto handRb = static_cast<hkpRigidBody*>(handRbBhk->referencedObject.get());
						if (handRb) {
							auto ms = handRb->GetMotionState();
							if (ms) {
								hkVector4 targetLocationHavok = hkVector4(targetLocation.x/70.0, targetLocation.y/70.0, targetLocation.z/70, 1.0);
								handRb->motion.SetPosition(targetLocationHavok);
								handRb->motion.SetLinearVelocity(hkVector4(0.0, 0.0, -10.0, 0.0));
							}
						}
					}
				}
			}
		}
	}

	void ChanceToScare(Actor* giant, Actor* tiny) {
		float sizedifference = GetSizeDifference(giant, tiny, true, true);
		if (sizedifference > 1.6 && !tiny->IsDead()) {
			int rng = rand() % 1600;
			rng /= sizedifference;
			if (rng <= 1.0 * sizedifference) {
				log::info("Trying to scare {}", tiny->GetDisplayFullName());
				bool IsScared = IsActionOnCooldown(tiny, CooldownSource::Action_ScareOther);
				log::info("Allow: {}", IsScared);
				if (!IsScared) {
					auto combat = tiny->GetActorRuntimeData().combatController;
					ApplyActionCooldown(tiny, CooldownSource::Action_ScareOther);
					if (!combat) {
						return;
					}
					auto cell = tiny->GetParentCell();
					if (cell) {
						auto TinyRef = skyrim_cast<TESObjectREFR*>(tiny);
						if (TinyRef) {
							auto GiantRef = skyrim_cast<TESObjectREFR*>(giant);
							if (GiantRef) {
								log::info("Scared {}", tiny->GetDisplayFullName());
								tiny->InitiateFlee(nullptr, false, true, false, nullptr, nullptr, 400.0, 465.0 * sizedifference);
							}
						}
					}
				}
			}
		}
	}

	void StaggerActor(Actor* receiver, float power) {
		if (receiver->IsDead() || IsRagdolled(receiver) || GetAV(receiver, ActorValue::kHealth) <= 0.0) {
			return;
		}
		receiver->SetGraphVariableFloat("staggerMagnitude", power);
		receiver->NotifyAnimationGraph("staggerStart");
	}

	void StaggerActor(Actor* giant, Actor* tiny, float power) {
		if (tiny->IsDead() || IsRagdolled(tiny) || GetAV(tiny, ActorValue::kHealth) <= 0.0) {
			return;
		}
		StaggerActor_Directional(giant, power, tiny);
	}

	void StaggerActor_Around(Actor* giant, const float radius) {
		if (!giant) {
			return;
		}
		auto node = find_node(giant, "NPC Root [Root]");
		if (!node) {
			return;
		}
		NiPoint3 NodePosition = node->world.translate;

		float giantScale = get_visual_scale(giant);
		float CheckDistance = radius * giantScale;

		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
			DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 600, {0.0, 1.0, 0.0, 1.0});
		}

		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				NiPoint3 actorLocation = otherActor->GetPosition();
				if ((actorLocation - giantLocation).Length() < CheckDistance*3) {
					int nodeCollisions = 0;
					float force = 0.0;

					auto model = otherActor->GetCurrent3D();

					if (model) {
						VisitNodes(model, [&nodeCollisions, &force, NodePosition, CheckDistance](NiAVObject& a_obj) {
							float distance = (NodePosition - a_obj.world.translate).Length();
							if (distance < CheckDistance) {
								nodeCollisions += 1;
								force = 1.0 - distance / CheckDistance;
								return false;
							}
							return true;
						});
					}
					if (nodeCollisions > 0) {
						StaggerActor(giant, otherActor, 0.50);
					}
				}
			}
		}
	}

	float GetMovementModifier(Actor* giant) {
		float modifier = 1.0;
		if (giant->AsActorState()->IsSprinting()) {
			modifier *= 1.33;
		}
		if (giant->AsActorState()->IsWalking()) {
			modifier *= 0.75;
		}
		if (giant->AsActorState()->IsSneaking()) {
			modifier *= 0.75;
		}
		return modifier;
	}

	float GetGtsSkillLevel() {
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		return GtsSkillLevel->value;
	}

	float GetXpBonus() {
		float xp = Persistent::GetSingleton().experience_mult;
		return xp;
	}

	float GetAttributePower() {
		float normal = 0.20;
		float level = std::clamp(GetGtsSkillLevel() * 0.008f, 80.0f, 0.0f);
		log::info("total boost: {}", normal + level);
		return normal + level;
	}

	void AddSMTDuration(Actor* actor, float duration) {
		if (!HasSMT(actor)) {
			return;
		}
		if (Runtime::HasPerk(actor, "EternalCalamity")) {
			auto transient = Transient::GetSingleton().GetData(actor);
			if (transient) {
				transient->SMT_Bonus_Duration += duration;
			}
		}
	}

	void AddSMTPenalty(Actor* actor, float penalty) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->SMT_Penalty_Duration += penalty;
		}
	}

	void PrintDeathSource(Actor* giant, Actor* tiny, DamageSource cause) {
		ReportDeath(giant, tiny, cause);
	}

	void PrintSuffocate(Actor* pred, Actor* prey) {
		int random = rand() % 6;
		if (random <= 1) {
			Cprint("{} was slowly smothered between {} thighs", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			Cprint("{} was suffocated by the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 3) {
			Cprint("Thighs of {} suffocated {} to death", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random == 4) {
			Cprint("{} got smothered between the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random >= 5) {
			Cprint("{} lost life to the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
	}

	void ShrinkUntil(Actor* giant, Actor* tiny, float expected, float halflife, bool animation) {
		if (HasSMT(giant)) {
			float Adjustment = GetSizeFromBoundingBox(tiny);
			float predscale = get_visual_scale(giant);
			float preyscale = get_target_scale(tiny) * Adjustment;
			float targetScale = predscale/expected;

			if (preyscale > targetScale) { // Apply ONLY if target is bigger than requirement

				if (animation) {
					Animation_TinyCalamity::AddToData(giant, tiny, expected);
					AnimationManager::StartAnim("Calamity_ShrinkOther", giant);
					return;
				}

				Task_AdjustHalfLifeTask(tiny, halflife); // to make them shrink faster

				set_target_scale(tiny, targetScale);
				StartCombat(tiny, giant);
				AddSMTPenalty(giant, 5.0 * Adjustment);
			}
		}
	}

	void DisableCollisions(Actor* actor, TESObjectREFR* otherActor) {
		if (actor) {
			auto trans = Transient::GetSingleton().GetData(actor);
			if (trans) {
				trans->disable_collision_with = otherActor;
				auto colliders = ActorCollisionData(actor);
				colliders.UpdateCollisionFilter();
				if (otherActor) {
					Actor* asOtherActor = skyrim_cast<Actor*>(otherActor);
					auto otherColliders = ActorCollisionData(asOtherActor);
					otherColliders.UpdateCollisionFilter();
				}
			}
		}
	}
	void EnableCollisions(Actor* actor) {
		if (actor) {
			auto trans = Transient::GetSingleton().GetData(actor);
			if (trans) {
				auto otherActor = trans->disable_collision_with;
				trans->disable_collision_with = nullptr;
				auto colliders = ActorCollisionData(actor);
				colliders.UpdateCollisionFilter();
				if (otherActor) {
					Actor* asOtherActor = skyrim_cast<Actor*>(otherActor);
					auto otherColliders = ActorCollisionData(asOtherActor);
					otherColliders.UpdateCollisionFilter();
				}
			}
		}
	}

	void SpringGrow(Actor* actor, float amt, float halfLife, std::string_view naming) {
		if (!actor) {
			return;
		}

		auto growData = std::make_shared<SpringGrowData>(actor, amt, halfLife);
		std::string name = std::format("SpringGrow {}: {}", naming, actor->formID);
		const float DURATION = halfLife * 3.2;

		TaskManager::RunFor(DURATION,
		                    [ growData ](const auto& progressData) {
			float totalScaleToAdd = growData->amount.value;
			float prevScaleAdded = growData->addedSoFar;
			float deltaScale = totalScaleToAdd - prevScaleAdded;
			Actor* actor = growData->actor.get().get();

			if (actor) {
				float stamina = clamp(0.05, 1.0, GetStaminaPercentage(actor));
				DamageAV(actor, ActorValue::kStamina, 0.55 * (get_visual_scale(actor) * 0.5 + 0.5) * stamina * TimeScale());
				auto actorData = Persistent::GetSingleton().GetData(actor);
				if (actorData) {
					actorData->target_scale += deltaScale;
					actorData->visual_scale += deltaScale;
					growData->addedSoFar = totalScaleToAdd;
				}
			}
			return fabs(growData->amount.value - growData->amount.target) > 1e-4;
		}
		                    );
	}

	void SpringGrow_Free(Actor* actor, float amt, float halfLife, std::string_view naming) {
		if (!actor) {
			return;
		}

		auto growData = std::make_shared<SpringGrowData>(actor, amt, halfLife);
		std::string name = std::format("SpringGrow_Free {}: {}", naming, actor->formID);
		const float DURATION = halfLife * 3.2;

		TaskManager::RunFor(name, DURATION,
		                    [ growData ](const auto& progressData) {
			float totalScaleToAdd = growData->amount.value;
			float prevScaleAdded = growData->addedSoFar;
			float deltaScale = totalScaleToAdd - prevScaleAdded;
			Actor* actor = growData->actor.get().get();

			if (actor) {
				auto actorData = Persistent::GetSingleton().GetData(actor);
				if (actorData) {
					actorData->target_scale += deltaScale;
					actorData->visual_scale += deltaScale;
					growData->addedSoFar = totalScaleToAdd;
				}
			}
			return fabs(growData->amount.value - growData->amount.target) > 1e-4;
		}
		                    );
	}

	void SpringShrink(Actor* actor, float amt, float halfLife, std::string_view naming) {
		if (!actor) {
			return;
		}

		auto growData = std::make_shared<SpringShrinkData>(actor, amt, halfLife);
		std::string name = std::format("SpringShrink {}: {}", naming, actor->formID);
		const float DURATION = halfLife * 3.2;
		TaskManager::RunFor(DURATION,
		                    [ growData ](const auto& progressData) {
			float totalScaleToAdd = growData->amount.value;
			float prevScaleAdded = growData->addedSoFar;
			float deltaScale = totalScaleToAdd - prevScaleAdded;
			Actor* actor = growData->actor.get().get();

			if (actor) {
				float stamina = clamp(0.05, 1.0, GetStaminaPercentage(actor));
				DamageAV(actor, ActorValue::kStamina, 0.35 * (get_visual_scale(actor) * 0.5 + 0.5) * stamina * TimeScale());
				auto actorData = Persistent::GetSingleton().GetData(actor);
				if (actorData) {
					actorData->target_scale += deltaScale;
					actorData->visual_scale += deltaScale;
					growData->addedSoFar = totalScaleToAdd;
				}
			}

			return fabs(growData->amount.value - growData->amount.target) > 1e-4;
		}
	);
	}

	void ResetGrab(Actor* giant) {
		if (giant->formID == 0x14 || IsTeammate(giant)) {
			AnimationManager::StartAnim("GrabAbort", giant); // Abort Grab animation
			AnimationManager::StartAnim("TinyDied", giant);

			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0); // Tell behaviors 'we have nothing in our hands'. A must.
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
			giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
			SetBetweenBreasts(giant, false);
		}
	}

	void FixAnimationsAndCamera() { // Fixes Animations for GTS Grab Actions and resets the bone tracking on camera
		auto profiler = Profilers::Profile("Utils: Actor State Fix");

		ResetCameraTracking(); // fix the camera tracking if loading previous save while voring/thigh crushing for example

		for (auto giant: find_actors()) {
			if (!giant) {
				continue;
			}
			int StateID;
			int GTSStateID;

			giant->GetGraphVariableInt("currentDefaultState", StateID);
			giant->GetGraphVariableInt("GTS_Def_State", GTSStateID);

			ResetGrab(giant);
			if (GTSStateID != StateID) {
				giant->SetGraphVariableInt("GTS_Def_State", StateID);
			}
		}
	}


	bool CanPerformAnimation(Actor* giant, float type) { // Needed for smooth animation unlocks during quest progression
		// 0 = Hugs
		// 1 = stomps and kicks
		// 2 = Grab and Sandwich
		// 3 = Vore
		// 5 = Others
		if (giant->formID != 0x14) {
			return true;
		} else {
			auto progressionQuest = Runtime::GetQuest("MainQuest");
			if (progressionQuest) {
				auto queststage = progressionQuest->GetCurrentStageID();
				if (queststage >= 10 && type == 0) {
					return true; // allow hugs
				} else if (queststage >= 30 && type == 1) {
					return true; // allow stomps and kicks
				} else if (queststage >= 50 && type == 2) {
					return true; // Allow grabbing and sandwiching
				} else if (queststage >= 60 && type >= 3) {
					return true; // Allow Vore
				} else if (queststage >= 100 && type >= 4) { 
					return true; // When quest is completed
				}
				else {
					return false;
				}
			}
			return false;
		}
	}

	void AdvanceQuestProgression(Actor* giant, float stage, float value) {
		if (giant->formID == 0x14) { // Player Only
			auto progressionQuest = Runtime::GetQuest("MainQuest");
			if (progressionQuest) {
				auto queststage = progressionQuest->GetCurrentStageID();
				if (queststage < 10 || queststage >= 100) {
					return;
				}
				if (stage == 1) {
					Persistent::GetSingleton().HugStealCount += value;
				} else if (stage == 2) {
					Persistent::GetSingleton().StolenSize += value;
				} else if (stage == 3 && queststage >= 30) {
					Persistent::GetSingleton().CrushCount += value;
				} else if (stage == 4 && queststage >= 40) {
					Persistent::GetSingleton().STNCount += value;
				} else if (stage == 5) {
					Persistent::GetSingleton().HandCrushed += value;
				} else if (stage == 6) {
					Persistent::GetSingleton().VoreCount += value;
				} else if (stage == 7) {
					Persistent::GetSingleton().GiantCount += value;
				}
			}
		}
	}

	void AdvanceQuestProgression(Actor* giant, Actor* tiny, float stage, float value, bool vore) {
		if (giant->formID == 0x14) { // Player Only
			auto progressionQuest = Runtime::GetQuest("MainQuest");
			if (progressionQuest) {
				auto queststage = progressionQuest->GetCurrentStageID();
				if (queststage < 10 || queststage >= 100) {
					return;
				}
				float bonus = 1.0;//GetSizeFromBoundingBox(tiny);
				if (stage == 1) {
					Persistent::GetSingleton().HugStealCount += value;
				} else if (stage == 2) {
					Persistent::GetSingleton().StolenSize += value;
				} else if (stage == 3 && queststage >= 30) {
					Persistent::GetSingleton().CrushCount += value * bonus;
					SpawnProgressionParticle(tiny, false);
				} else if (stage == 4 && queststage >= 40) {
					Persistent::GetSingleton().STNCount += value * bonus;
					SpawnProgressionParticle(tiny, false);
				} else if (stage == 5) {
					Persistent::GetSingleton().HandCrushed += value * bonus;
					SpawnProgressionParticle(tiny, false);
				} else if (stage == 6) {
					Persistent::GetSingleton().VoreCount += value * bonus;
					SpawnProgressionParticle(tiny, true);
				} else if (stage == 7) {
					Persistent::GetSingleton().GiantCount += value;
					SpawnProgressionParticle(tiny, vore);
				}
			}
		}
	}

	void ResetQuest() {
		Persistent::GetSingleton().HugStealCount = 0.0;
		Persistent::GetSingleton().StolenSize = 0.0;
		Persistent::GetSingleton().CrushCount = 0.0;
		Persistent::GetSingleton().STNCount = 0.0;
		Persistent::GetSingleton().HandCrushed = 0.0;
		Persistent::GetSingleton().VoreCount = 0.0;
		Persistent::GetSingleton().GiantCount = 0.0;
	}


	void SpawnProgressionParticle(Actor* tiny, bool vore) {
		float scale = 1.0 * GetSizeFromBoundingBox(tiny);

		if (tiny->IsDead()) {
			scale *= 0.33;
		}

		auto node = find_node(tiny, "NPC Root [Root]");
		log::info("Spawning particle");
		if (node) {
			NiPoint3 pos = node->world.translate;
			if (!vore) {
				SpawnParticle(tiny, 4.60, "GTS/Magic/Life_Drain.nif", NiMatrix3(), pos, scale, 7, nullptr);
				log::info("Soul false, spawning particle");
			} else {
				SpawnParticle(tiny, 4.60, "GTS/Magic/Soul_Drain.nif", NiMatrix3(), pos, scale, 7, nullptr);
				log::info("Soul true, spawning particle");
			}
		}
	}

	float GetQuestProgression(float stage) {
		if (stage == 1) {
			return Persistent::GetSingleton().HugStealCount;
		} else if (stage == 2) {
			return Persistent::GetSingleton().StolenSize;
		} else if (stage == 3) {
			return Persistent::GetSingleton().CrushCount;
		} else if (stage == 4) {
			return (Persistent::GetSingleton().CrushCount - 3.0) + Persistent::GetSingleton().STNCount;
		} else if (stage == 5) {
			return Persistent::GetSingleton().HandCrushed;
		} else if (stage == 6) {
			return Persistent::GetSingleton().VoreCount;
		} else if (stage == 7) {
			return Persistent::GetSingleton().GiantCount;
		} else {
			return 0.0;
		}
	}

	void InflictSizeDamage(Actor* attacker, Actor* receiver, float value) {
		if (!receiver->IsDead()) {
			float HpPercentage = GetHealthPercentage(receiver);
			float difficulty = 2.0; // taking Legendary Difficulty as a base
			float levelbonus = 1.0 + ((GetGtsSkillLevel() * 0.01) * 0.50);
			value *= levelbonus;

			if (HpPercentage < 0.70) { // Mostly a warning to indicate that actor dislikes it (They don't always aggro right away, with mods at least)
				Attacked(receiver, attacker);
			} 
			if (value >= GetAV(receiver, ActorValue::kHealth) * 0.50) { // in that case make hostile
				if (!IsTeammate(receiver) && !IsHostile(attacker, receiver)) {
					StartCombat(receiver, attacker); // Make actor hostile and add bounty of 40 (can't be configured, needs different hook probably). 
				}
			}
			 
			ApplyDamage(attacker, receiver, value * difficulty * GetDamageSetting());
		} else if (receiver->IsDead() || GetAV(receiver, ActorValue::kHealth) < 0.0) {
			receiver->InitHavok();
			// ^ Needed to fix this issue:
			//   https://www.reddit.com/r/skyrimmods/comments/402b69/help_looking_for_a_bugfix_dead_enemies_running_in/
		}

	}

	float Sound_GetFallOff(NiAVObject* source, float mult) {
		if (source) {
			float distance_to_camera = unit_to_meter(get_distance_to_camera(source));
			// Camera distance based volume falloff
			return soft_core(distance_to_camera, 0.024 / mult, 2.0, 0.8, 0.0, 0.0);
		}
		return 1.0;
	} 

	// RE Fun
	void SetCriticalStage(Actor* actor, int stage) {
		if (stage < 5 && stage >= 0) {
			typedef void (*DefSetCriticalStage)(Actor* actor, int stage);
			REL::Relocation<DefSetCriticalStage> SkyrimSetCriticalStage{ RELOCATION_ID(36607, 37615) };
			SkyrimSetCriticalStage(actor, stage);
		}
	}
	void Attacked(Actor* victim, Actor* agressor) {
		typedef void (*DefAttacked)(Actor* victim, Actor* agressor);
		REL::Relocation<DefAttacked> SkyrimAttacked{ RELOCATION_ID(37672, 38626) }; // 6285A0 (SE) ; 64EE60 (AE)
		SkyrimAttacked(victim, agressor); 
	}

	void StartCombat(Actor* victim, Actor* agressor) {
		// This function starts combat and adds bounty. Sadly adds 40 bounty since it's not a murder, needs other hook for murder bounty.
		typedef void (*DefStartCombat)(Actor* act_1, Actor* act_2, Actor* act_3, Actor* act_4);
		REL::Relocation<DefStartCombat> SkyrimStartCombat{ RELOCATION_ID(36430, 37425) }; // sub_1405DE870 : 36430  (SE) ; 1406050c0 : 37425 (AE)
		SkyrimStartCombat(victim, agressor, agressor, agressor);                          // Called from Attacked above at some point
	}

	void ApplyDamage(Actor* giant, Actor* tiny, float damage) { // applies correct amount of damage and kills actors properly
		typedef void (*DefApplyDamage)(Actor* a_this, float dmg, Actor* aggressor, HitData* maybe_hitdata, TESObjectREFR* damageSrc);
		REL::Relocation<DefApplyDamage> Skyrim_ApplyDamage{ RELOCATION_ID(36345, 37335) }; // 5D6300 (SE)
		Skyrim_ApplyDamage(tiny, damage, nullptr, nullptr, nullptr);
	}

	void SetObjectRotation_X(TESObjectREFR* ref, float X) {
		typedef void (*DefSetRotX)(TESObjectREFR* ref, float rotation);
		REL::Relocation<DefSetRotX> SetObjectRotation_X{ RELOCATION_ID(19360, 19360) }; // 140296680 (SE)
		SetObjectRotation_X(ref, X);
	}

	void StaggerActor_Directional(Actor* giant, float power, Actor* tiny) {
		//SE: 1405FA1B0 : 36700 (Character *param_1,float param_2,Actor *param_3)
		//AE: 140621d80 : 37710
		//log::info("Performing Directional Stagger");
		//log::info("Giant: {}, Tiny: {}, Power: {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), power);
		typedef void (*DefStaggerActor_Directional)(Actor* tiny, float power, Actor* giant);
		REL::Relocation<DefStaggerActor_Directional> StaggerActor_Directional{ RELOCATION_ID(36700, 37710) };
		StaggerActor_Directional(tiny, power, giant);
	}

	std::int16_t GetItemCount(InventoryChanges* changes, RE::TESBoundObject* a_obj)
	{
		using func_t = decltype(&GetItemCount);
		REL::Relocation<func_t> func{ RELOCATION_ID(15868, 16108) };
		return func(changes, a_obj);
	}

	int GetCombatState(Actor* actor) { // Needs AE address
        using func_t = decltype(GetCombatState);
        REL::Relocation<func_t> func{ RELOCATION_ID(37603, 37603) };
        return func(actor);
		// 0 = non combat, 1 = combat, 2 = search
    }

	bool IsMoving(Actor* giant) { // New CommonLib version copy-paste
		using func_t = decltype(&IsMoving);
		REL::Relocation<func_t> func{ RELOCATION_ID(36928, 37953) };
		return func(giant);
	}
}
