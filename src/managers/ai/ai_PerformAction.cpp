#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/ai_PerformAction.hpp"
#include "managers/GtsSizeManager.hpp"
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
        if (rng <= 3) {
            AnimationManager::StartAnim("SwipeHeavy_Right", pred);
        } else if (rng <= 4) {
            AnimationManager::StartAnim("SwipeHeavy_Left", pred);
        } else if (rng <= 6) {
            AnimationManager::StartAnim("SwipeLight_Left", pred);
        } else {
            AnimationManager::StartAnim("SwipeLight_Right", pred);
        }
    }

    void AI_FastButtCrush(Actor* pred) { // we do not support manual butt crush because it requires additional logic
        if (!Persistent::GetSingleton().Butt_Ai) {
            return;
        }
        AnimationManager::StartAnim("ButtCrush_StartFast", pred);
    }
}