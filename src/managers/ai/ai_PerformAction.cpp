#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/ai_PerformAction.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"
namespace {
    const std::vector<std::string_view> light_kicks = {
        "SwipeLight_Left",                  // 0
        "SwipeLight_Right",                 // 1
        "SwipeLight_Right",                 // 2, a fail-safe one in case random does funny stuff 
    };

    const std::vector<std::string_view> heavy_kicks = {
        "SwipeHeavy_Right",                 // 0
        "SwipeHeavy_Left",                  // 1
        "StrongKick_Low_Right",             // 2
        "StrongKick_Low_Left",              // 3
        "StrongKick_Low_Left",              // 4, a fail-safe one in case random does funny stuff 
    };

    void Task_ButtCrushLogicTask(Actor* giant) {

        std::string name = std::format("ButtCrush_AI_{}", giant->formID);

        auto gianthandle = giant->CreateRefHandle();
		auto FrameA = Time::FramesElapsed();
		TaskManager::Run(name, [=](auto& progressData) {
            if (!gianthandle) {
                return false;
            }
            auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 10.0) {
				return true;
			}

            auto giantref = gianthandle.get().get();

            bool CanGrow = ButtCrush_IsAbleToGrow(giantref, GetGrowthLimit(giantref));

            bool BlockGrowth = IsActionOnCooldown(giantref, CooldownSource::Misc_AiGrowth);

            if (IsChangingSize(giantref)) { // Growing/shrinking
                ApplyActionCooldown(giantref, CooldownSource::Misc_AiGrowth);
            }

            if (BlockGrowth) {
                return true;
            }
            
            if (CanGrow && IsButtCrushing(giantref) && !IsChangingSize(giantref) && Runtime::HasPerkTeam(giantref, "ButtCrush_GrowingDisaster")) {
                ApplyActionCooldown(giantref, CooldownSource::Misc_AiGrowth);
                int rng = rand()% 10;
                if (rng <= 6) {
                    AnimationManager::StartAnim("ButtCrush_Growth", giantref);
                }
            } else if (!CanGrow) { // Can't grow any further
                AnimationManager::StartAnim("ButtCrush_Attack", giantref);
            }

            if (!IsButtCrushing(giantref)) {
                return false; // End the task
            }
            return true;
        });
    }
    

    void AI_Heavy_Kicks(Actor* pred) {
        int rng = rand() % 4;
        int limit = 3;
        if (IsCrawling(pred)) {
            limit = 1;
            rng = rand() % 2;
        }  
        if (rng > limit) {
            rng = limit; // fail-safe thingie
        }
        log::info("Heavy Kicks rng for {} is {}", pred->GetDisplayFullName(), rng);
        AnimationManager::StartAnim(heavy_kicks[rng], pred);
    }
    void AI_Light_Kicks(Actor* pred) {
        int rng = rand() % 2;
        int limit = 1;
        if (rng > limit) {
            rng = limit; // fail-safe thingie
        }
        //log::info("Light Kicks rng for {} is {}", pred->GetDisplayFullName(), rng);
        AnimationManager::StartAnim(light_kicks[rng], pred);
    }
}

namespace Gts {
    
    void AI_StrongStomp(Actor* pred, int rng) {
        if (!Persistent::GetSingleton().Stomp_Ai) {
            return; // don't check any further if it is disabled
        }
        if (rng <= 5) {
            AnimationManager::StartAnim("StrongStompRight", pred);
        } else {
            AnimationManager::StartAnim("StrongStompLeft", pred);
        }
    }
    void AI_LightStomp(Actor* pred, int rng) {
        if (!Persistent::GetSingleton().Stomp_Ai) {
            return; // don't check any further if it is disabled
        }
        Utils_UpdateHighHeelBlend(pred, false);
        if (rng <= 5) {
            AnimationManager::StartAnim("StompRight", pred);
        } else {
            AnimationManager::StartAnim("StompLeft", pred);
        }
    }

    void AI_Tramples(Actor* pred, int rng) {
        if (!Persistent::GetSingleton().Stomp_Ai) {
            return;
        }
        Utils_UpdateHighHeelBlend(pred, false);
        if (rng <= 5) {
            AnimationManager::StartAnim("TrampleL", pred);
        } else {
            AnimationManager::StartAnim("TrampleR", pred);
        }
    }

    void AI_Kicks(Actor* pred, int rng) {
        if (!Persistent::GetSingleton().Kick_Ai) {
            return;
        }
        Utils_UpdateHighHeelBlend(pred, false);
        if (rng <= 5) {
            AI_Heavy_Kicks(pred);
        } else {
            AI_Light_Kicks(pred);
        }
    }

    void AI_ButtCrush(Actor* pred, Actor* prey) {
        if (!Persistent::GetSingleton().Butt_Ai) {
            return;
        }
        if (IsActionOnCooldown(pred, CooldownSource::Action_ButtCrush)) {
            return;
        }
        if (IsGtsBusy(pred) || IsChangingSize(pred)) {
            return;
        }
        auto grabbedActor = Grab::GetHeldActor(pred);
        if (grabbedActor && !IsCrawling(pred)) { // If gts has someone in hands, allow only when we crawl
            return;
        }

        int rng = rand() % 10;
        if (Runtime::HasPerkTeam(pred, "ButtCrush_NoEscape") && rng > 2) {
            auto& ButtCrush = ButtCrushController::GetSingleton();

            ButtCrush.StartButtCrush(pred, prey); // attaches actors to AnimObjectB
            Task_ButtCrushLogicTask(pred);
        } else {
            AnimationManager::StartAnim("ButtCrush_StartFast", pred);
        }
    }
}