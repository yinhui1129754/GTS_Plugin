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
    bool CanForceCrush(Actor* giant, Actor* huggedActor) {
        bool ForceCrush = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles");
        float staminapercent = GetStaminaPercentage(giant);
        float stamina = GetAV(giant, ActorValue::kStamina);
        if (ForceCrush && staminapercent >= 0.50) {
            AnimationManager::StartAnim("Cleavage_Absorb", giant);
            DamageAV(giant, ActorValue::kStamina, stamina * 1.10);
            return true;
        }
        return false;
    }

    void AttemptAbsorption(bool force) {
        Actor* player = GetPlayerOrControlled();
        if (IsInCleavageState(player)) {
            auto tiny = Grab::GetHeldActor(player);
            if (tiny) {
                float HpThreshold = GetHugCrushThreshold(player, tiny) * 0.2;
                float health = GetHealthPercentage(tiny);
                if (health <= HpThreshold) {
                    AnimationManager::StartAnim("Cleavage_Absorb", player);
                    return;
                } else if (HasSMT(player)) {
                    DamageAV(player, ActorValue::kStamina, 60);
                    AnimationManager::StartAnim("Cleavage_Absorb", player);
                    AddSMTPenalty(player, 10.0);
                    return;
                } else {
                    if (CanForceCrush(player, tiny)) {
                        return;
                    }
                    std::string message = std::format("{} is too healthy to be absorbed by breasts", tiny->GetDisplayFullName());
                    shake_camera(player, 0.45, 0.30);
                    TiredSound(player, message);

                    Notify("Health: {:.0f}%; Requirement: {:.0f}%", health * 100.0, HpThreshold * 100.0);
                }
            }
        } 
    }
    void PassAnimation(std::string animation, bool check_cleavage) {
        Actor* player = GetPlayerOrControlled();
        if (player) {
            bool BetweenCleavage = IsInCleavageState(player);
            if (BetweenCleavage || !check_cleavage) {
                AnimationManager::StartAnim(animation, player);
            }
        }
    }

    void CleavageEnterEvent(const InputEventData& data) {
        Actor* giant = PlayerCharacter::GetSingleton();
        if (giant) {
            Actor* tiny = Grab::GetHeldActor(giant);
            if (tiny && IsBetweenBreasts(tiny)) {
                PassAnimation("Cleavage_EnterState", false);
            }
        }
    }
    void CleavageExitEvent(const InputEventData& data) {
        PassAnimation("Cleavage_ExitState", true);
    }
    void ClevageLightAttackEvent(const InputEventData& data) {
        PassAnimation("Cleavage_LightAttack", true);
    }
    void ClevageHeavyAttackEvent(const InputEventData& data) {
        PassAnimation("Cleavage_HeavyAttack", true);
    }
    void CleavageForceAbsorbEvent(const InputEventData& data) {
        AttemptAbsorption(true);
    }
    void ClevageAbsorbEvent(const InputEventData& data) {
        AttemptAbsorption(false);
    }
    void ClevageVoreEvent(const InputEventData& data) {
        PassAnimation("Cleavage_Vore", true);
    }
}

namespace Gts
{
	void Animation_Cleavage::RegisterEvents() {
        InputManager::RegisterInputEvent("CleavageEnter", CleavageEnterEvent);
        InputManager::RegisterInputEvent("CleavageExit", CleavageExitEvent);
        InputManager::RegisterInputEvent("CleavageLightAttack", ClevageLightAttackEvent);
        InputManager::RegisterInputEvent("CleavageHeavyAttack", ClevageHeavyAttackEvent);
        InputManager::RegisterInputEvent("CleavageAbsorb", ClevageAbsorbEvent);
        InputManager::RegisterInputEvent("CleavageVore", ClevageVoreEvent);
	}

	void Animation_Cleavage::RegisterTriggers() {
        AnimationManager::RegisterTrigger("Cleavage_EnterState", "Cleavage", "GTSBEH_Boobs_Enter");
        AnimationManager::RegisterTrigger("Cleavage_ExitState", "Cleavage", "GTSBEH_Boobs_Exit");
		AnimationManager::RegisterTrigger("Cleavage_LightAttack", "Cleavage", "GTSBEH_Boobs_Crush_Light");
        AnimationManager::RegisterTrigger("Cleavage_HeavyAttack", "Cleavage", "GTSBEH_Boobs_Crush_Heavy");
        AnimationManager::RegisterTrigger("Cleavage_Absorb", "Cleavage", "GTSBEH_Boobs_Absorb");
        AnimationManager::RegisterTrigger("Cleavage_Abort", "Cleavage", "GTSBEH_Boobs_Abort");
        AnimationManager::RegisterTrigger("Cleavage_Vore", "Cleavage", "GTSBEH_Boobs_Vore");
	}
}