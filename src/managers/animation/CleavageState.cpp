#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/CleavageState.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

/*
GTS events:
GTSBEH_Boobs_Enter
GTSBEH_Boobs_Exit
GTSBEH_Boobs_Crush_Light
GTSBEH_Boobs_Crush_Heavy
GTSBEH_Boobs_Vore
GTSBEH_Boobs_Absorb
GTSBEH_Boobs_Abort

TIny Events
GTSBEH_T_Boobs_Enter
GTSBEh_T_Boobs_Exit
GTSBEh_T_Boobs_Crush_Light
GTSBEh_T_Boobs_Crush_Heavy
GTSBEh_T_Boobs_Vore
GTSBEh_T_Boobs_Absorb 


IsInCleavageState(Actor* actor)
*/

namespace {
    const std::vector<std::string_view> BREAST_NODES_R = { // used for body rumble
        "R Breast01",
        "R Breast02",
		"R Breast03",
        "R Breast04",
	};

    const std::vector<std::string_view> BREAST_NODES_L = { // used for body rumble
        "L Breast00",
        "L Breast01",
        "L Breast02",
		"L Breast03",
        "L Breast04",
	};

    bool CanForceAction(Actor* giant, Actor* huggedActor, std::string pass_anim) {
        bool ForceCrush = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles");
        float staminapercent = GetStaminaPercentage(giant);
        float stamina = GetAV(giant, ActorValue::kStamina);
        if (ForceCrush && staminapercent >= 0.50) {
            AnimationManager::StartAnim(pass_anim, giant);
            DamageAV(giant, ActorValue::kStamina, stamina * 1.10);
            return true;
        }
        return false;
    }

    float GetMasteryReduction(Actor* giant) {
        float cooldown = 0.0;
        if (Runtime::HasPerk(giant, "Breasts_MasteryPart2")) {
            cooldown = GetGtsSkillLevel(giant) * 0.006;
        }

        return cooldown;
    }
    void AttemptBreastActionOnTiny(std::string pass_anim) {
        Actor* player = GetPlayerOrControlled();
        if (IsInCleavageState(player)) {
            auto tiny = Grab::GetHeldActor(player);
            if (tiny) {
                AnimationManager::StartAnim(pass_anim, tiny);
            }
        }
    }
    bool AttemptBreastAction(std::string pass_anim, CooldownSource Source, std::string cooldown_msg, std::string perk) {
        Actor* player = GetPlayerOrControlled();
        if (IsInCleavageState(player)) {
            auto tiny = Grab::GetHeldActor(player);
            if (tiny) {
                bool OnCooldown = IsActionOnCooldown(player, Source);
                if (!OnCooldown) {
                    if (Runtime::HasPerk(player, perk)) {
                        float HpThreshold = (GetHugCrushThreshold(player, tiny, false) * 1.5) + GetMasteryReduction(player);
                        float health = GetHealthPercentage(tiny);
                        if (health <= HpThreshold) {
                            AnimationManager::StartAnim(pass_anim, player);
                            return true;
                        } else if (HasSMT(player)) {
                            DamageAV(player, ActorValue::kStamina, 60);
                            AnimationManager::StartAnim(pass_anim, player);
                            AddSMTPenalty(player, 10.0);
                            return true;
                        } else {
                            if (CanForceAction(player, tiny, pass_anim)) {
                                return true;
                            }
                            std::string message = std::format("{} is too healthy for {}", tiny->GetDisplayFullName(), cooldown_msg);
                            shake_camera(player, 0.45, 0.30);
                            TiredSound(player, message);

                            Notify("Health: {:.0f}%; Requirement: {:.0f}%", health * 100.0, HpThreshold * 100.0);
                            return false;
                        }
                    }
                } else {
                    std::string message = std::format("{} is on a cooldown: {:.1f} sec", cooldown_msg, GetRemainingCooldown(player, Source));
                    shake_camera(player, 0.45, 0.30);
                    TiredSound(player, message);
                    return false;
                }
            }
        } 

        return false;
    }
    bool PassAnimation(std::string animation, bool check_cleavage) {
        Actor* player = GetPlayerOrControlled();
        if (player) {
            bool BetweenCleavage = IsInCleavageState(player);
            if (BetweenCleavage || !check_cleavage) {
                AnimationManager::StartAnim(animation, player);
                return true;
            }
        }
        return false;
    }

    void CleavageEnterEvent(const InputEventData& data) {
        Actor* giant = PlayerCharacter::GetSingleton();
        if (giant) {
            Actor* tiny = Grab::GetHeldActor(giant);
            if (tiny && IsBetweenBreasts(tiny)) {
                PassAnimation("Cleavage_EnterState", false);
                AttemptBreastActionOnTiny("Cleavage_EnterState_Tiny");
            }
        }
    }
    void CleavageExitEvent(const InputEventData& data) {
        PassAnimation("Cleavage_ExitState", true);
    }
    void CleavageLightAttackEvent(const InputEventData& data) {
        if (PassAnimation("Cleavage_LightAttack", true)) {
            AttemptBreastActionOnTiny("Cleavage_LightAttack_Tiny");
        }
    }
    void CleavageHeavyAttackEvent(const InputEventData& data) {
        if (PassAnimation("Cleavage_HeavyAttack", true)) {
            AttemptBreastActionOnTiny("Cleavage_HeavyAttack_Tiny");
        }
    }
    void CleavageSuffocateEvent(const InputEventData& data) {
        if (AttemptBreastAction("Cleavage_Suffocate", CooldownSource::Action_Breasts_Suffocate, "Suffocation", "Breasts_Suffocate")) {
            AttemptBreastActionOnTiny("Cleavage_Suffocate_Tiny");
        }
    }
    void CleavageAbsorbEvent(const InputEventData& data) {
        if (AttemptBreastAction("Cleavage_Absorb", CooldownSource::Action_Breasts_Absorb, "Absorption", "Breasts_Absorb")) {
            AttemptBreastActionOnTiny("Cleavage_Absorb_Tiny");
        }
    }
    void CleavageVoreEvent(const InputEventData& data) {
        if (AttemptBreastAction("Cleavage_Vore", CooldownSource::Action_Breasts_Vore, "Vore", "Breasts_Vore")) {
            AttemptBreastActionOnTiny("Cleavage_Vore_Tiny");
        }
    }
}

namespace Gts
{
    void Animation_Cleavage::LaunchCooldownFor(Actor* giant, CooldownSource Source) {
        std::string name = std::format("CDWatcher_{}_{}", giant->formID, Time::WorldTimeElapsed());
        ActorHandle gianthandle = giant->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

            if (!IsInCleavageState(giantref)) {
                return false;
            }

            ApplyActionCooldown(giant, Source);

			return true;
		});
    }

	void Animation_Cleavage::RegisterEvents() {
        InputManager::RegisterInputEvent("CleavageEnter", CleavageEnterEvent);
        InputManager::RegisterInputEvent("CleavageExit", CleavageExitEvent);
        InputManager::RegisterInputEvent("CleavageLightAttack", CleavageLightAttackEvent);
        InputManager::RegisterInputEvent("CleavageHeavyAttack", CleavageHeavyAttackEvent);
        InputManager::RegisterInputEvent("CleavageSuffocate", CleavageSuffocateEvent);
        InputManager::RegisterInputEvent("CleavageAbsorb", CleavageAbsorbEvent);
        InputManager::RegisterInputEvent("CleavageVore", CleavageVoreEvent);
	}

	void Animation_Cleavage::RegisterTriggers() {
        
        AnimationManager::RegisterTrigger("Cleavage_EnterState_Tiny", "Cleavage", "GTSBEH_T_Boobs_Enter");
        AnimationManager::RegisterTrigger("Cleavage_EnterState", "Cleavage", "GTSBEH_Boobs_Enter");

        AnimationManager::RegisterTrigger("Cleavage_ExitState", "Cleavage", "GTSBEH_Boobs_Exit");

        AnimationManager::RegisterTrigger("Cleavage_LightAttack_Tiny", "Cleavage", "GTSBEH_T_Boobs_Crush_Light");
		AnimationManager::RegisterTrigger("Cleavage_LightAttack", "Cleavage", "GTSBEH_Boobs_Crush_Light");

        AnimationManager::RegisterTrigger("Cleavage_HeavyAttack_Tiny", "Cleavage", "GTSBEH_T_Boobs_Crush_Heavy");
        AnimationManager::RegisterTrigger("Cleavage_HeavyAttack", "Cleavage", "GTSBEH_Boobs_Crush_Heavy");

        AnimationManager::RegisterTrigger("Cleavage_SuffocateStop", "Cleavage", "GTSBEH_Boobs_SufoStop");

        AnimationManager::RegisterTrigger("Cleavage_Suffocate_Tiny", "Cleavage", "GTSBEH_T_Boobs_SufoStart");
        AnimationManager::RegisterTrigger("Cleavage_Suffocate", "Cleavage", "GTSBEH_Boobs_SufoStart");

        AnimationManager::RegisterTrigger("Cleavage_Absorb_Tiny", "Cleavage", "GTSBEH_T_Boobs_Absorb");
        AnimationManager::RegisterTrigger("Cleavage_Absorb", "Cleavage", "GTSBEH_Boobs_Absorb");

        AnimationManager::RegisterTrigger("Cleavage_Abort", "Cleavage", "GTSBEH_Boobs_Abort");

        AnimationManager::RegisterTrigger("Cleavage_Vore_Tiny", "Cleavage", "GTSBEH_T_Boobs_Vore");
        AnimationManager::RegisterTrigger("Cleavage_Vore", "Cleavage", "GTSBEH_Boobs_Vore");


        /* Tiny Events:
        GTSBEH_T_Boobs_Enter
        GTSBEH_T_Boobs_Exit
        GTSBEH_T_Boobs_Crush_Light
        GTSBEH_T_Boobs_Crush_Heavy
        GTSBEH_T_Boobs_Vore
        GTSBEH_T_Boobs_Absorb
        GTSBEH_T_Boobs_SufoStart
        GTSBEH_T_Boobs_SufoStop
        */
	}
}