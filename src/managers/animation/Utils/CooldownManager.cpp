#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>


using namespace Gts;
using namespace RE;
using namespace REL;
using namespace SKSE;


namespace {
	const double LAUNCH_COOLDOWN = 1.8f;
	const double PUSH_COOLDOWN = 2.0f;
	const double HANDDAMAGE_COOLDOWN = 0.6f;
	const double THIGHDAMAGE_COOLDOWN = 1.2f;

    const double ABSORB_OTHER_COOLDOWN = 30.0f;

    const double BREAST_SUFFOCATE_OTHER_COOLDOWN = 30.0f;
    const double BREAST_ABSORB_OTHER_COOLDOWN = 30.0f;
    const double BREAST_VORE_OTHER_COOLDOWN = 30.0f;

    const double TINYCALAMITY_ONESHOT_COOLDOWN = 60.0f;
    
	const double HEALTHGATE_COOLDOWN = 60.0f;
	const double SCARE_COOLDOWN = 6.0f;
	const double BUTTCRUSH_COOLDOWN = 30.0f;
	const double HUGS_COOLDOWN = 10.0f;

    const double LAUGH_COOLDOWN = 5.0f;
	const double MOAN_COOLDOWN = 5.0f;

    const double SOUND_COOLDOWN = 2.0f;
    const double GROW_SOUND_COOLDOWN = 1.0f;

    const double HIT_COOLDOWN = 1.0f;
    const double AI_GROWTH_COOLDOWN = 2.0f;
    const double SHRINK_OUTBURST_COOLDOWN = 18.0f;
    const double SHRINK_OUTBURST_COOLDOWN_FORCED = 180.0f;

    float Calculate_BreastActionCooldown(Actor* giant, int type) {
        float Cooldown = 1.0;
        float mastery = 0.0;
        switch (type) {
            case 0:
                Cooldown = BREAST_SUFFOCATE_OTHER_COOLDOWN;
            break;
            case 1:
                Cooldown = BREAST_VORE_OTHER_COOLDOWN;
            break;
            case 2:
                Cooldown = BREAST_ABSORB_OTHER_COOLDOWN;
            break;
            default: 
                Cooldown = BREAST_SUFFOCATE_OTHER_COOLDOWN;
            break;   
        }

        if (Runtime::HasPerk(giant, "Breasts_Mastery")) {
            mastery = std::clamp(GetGtsSkillLevel(giant) * 0.01f, 0.0f, 1.0f) * 0.6;
        }
        float reduction = 1.0 - mastery;

        return Cooldown * reduction;
    }

    float Calculate_AbsorbCooldown(Actor* giant) {
        float mastery = std::clamp(GetGtsSkillLevel(giant) * 0.01f, 0.0f, 1.0f) * 0.73;
        float reduction = 1.0 - mastery; // Up to 8.1 seconds at level 100

        log::info("Mastery of {} is {}", giant->GetDisplayFullName(), mastery); 
        log::info("Total Cooldown: {}", ABSORB_OTHER_COOLDOWN * reduction);

        return ABSORB_OTHER_COOLDOWN * reduction;
    }

    float Calculate_ButtCrushTimer(Actor* actor) {
		bool lvl70 = Runtime::HasPerk(actor, "ButtCrush_UnstableGrowth");
		bool lvl100 = Runtime::HasPerk(actor, "ButtCrush_LoomingDoom");
		float reduction = 1.0;
		if (lvl100) { // 15% reduction
			reduction -= 0.15;
		} if (lvl70) { // 10% reduction
			reduction -= 0.10;
		} 
		return BUTTCRUSH_COOLDOWN * reduction;
	}

    float Calculate_FootstepTimer(Actor* actor) {
        float cooldown = 0.2;
        cooldown /= AnimationManager::GetAnimSpeed(actor);
        //log::info("Cooldown for footstep: {}", cooldown);
        return cooldown;
    }

    float Calculate_ShrinkOutbirstTimer(Actor* actor) {
        bool DarkArts3 = Runtime::HasPerk(actor, "DarkArts_Aug3");
        bool HealthRegen = Runtime::HasPerk(actor, "HealthRegenPerk");
        float reduction = 1.0;
        if (DarkArts3) {
            reduction = 0.7;
        }
        if (HealthRegen && IsGrowthSpurtActive(actor)) {
            reduction *= 0.75;
        }
        return SHRINK_OUTBURST_COOLDOWN * reduction;
    }
}

namespace Gts {

    CooldownManager& CooldownManager::GetSingleton() noexcept {
		static CooldownManager instance;
		return instance;
	}

	std::string CooldownManager::DebugName() {
		return "CooldownManager";
	}

    CooldownData& CooldownManager::GetCooldownData(Actor* actor) {
		this->CooldownData.try_emplace(actor);
		return this->CooldownData.at(actor);
	}

    void CooldownManager::Reset() {
        this->CooldownData.clear();
        log::info("Cooldowns cleared");
    }

    void ApplyActionCooldown(Actor* giant, CooldownSource source) {
        float time = Time::WorldTimeElapsed();
        auto& data = CooldownManager::GetSingleton().GetCooldownData(giant);

        switch (source) {
            case CooldownSource::Damage_Launch: 
                data.lastLaunchTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Damage_Hand:
                data.lastHandDamageTime = Time::WorldTimeElapsed();
                break;    
            case CooldownSource::Damage_Thigh:
                data.lastThighDamageTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Push_Basic:
                data.lastPushTime = Time::WorldTimeElapsed();
                break;    
            case CooldownSource::Action_ButtCrush:
                data.lastButtCrushTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Action_HealthGate:
                data.lastHealthGateTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Action_ScareOther:   
                data.lastScareTime = Time::WorldTimeElapsed();
                break; 
            case CooldownSource::Action_Hugs:   
                data.lastHugTime = Time::WorldTimeElapsed();
                break;     
            case CooldownSource::Action_AbsorbOther:
                data.lastAbsorbTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Action_Breasts_Suffocate:
                data.lastBreastSuffocateTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Action_Breasts_Vore:
                data.lastBreastVoreTime = Time::WorldTimeElapsed();
                break;   
            case CooldownSource::Action_Breasts_Absorb:
                data.lastBreastAbsorbTime = Time::WorldTimeElapsed();
                break;        
            case CooldownSource::Emotion_Laugh:   
                data.lastLaughTime = Time::WorldTimeElapsed();
                break; 
            case CooldownSource::Emotion_Moan: 
                data.lastMoanTime = Time::WorldTimeElapsed();
                break;  
            case CooldownSource::Misc_RevertSound: 
                data.lastRevertTime = Time::WorldTimeElapsed();
                break; 
            case CooldownSource::Misc_GrowthSound:
                data.lastSoundGrowthTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Misc_BeingHit:
                data.lastHitTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Misc_AiGrowth:
                data.lastGrowthTime = Time::WorldTimeElapsed();
                break;    
            case CooldownSource::Misc_ShrinkOutburst:
                data.lastOutburstTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Misc_ShrinkOutburst_Forced:
                data.lastForceOutburstTime = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Footstep_Right:
                data.lastFootstepTime_R = Time::WorldTimeElapsed();
                break;
            case CooldownSource::Footstep_Left:
                data.lastFootstepTime_L = Time::WorldTimeElapsed();
                break; 
        }
    }

    float GetRemainingCooldown(Actor* giant, CooldownSource source) {
        float time = Time::WorldTimeElapsed();
        auto& data = CooldownManager::GetSingleton().GetCooldownData(giant);

        switch (source) {
            case CooldownSource::Damage_Launch: 
                return (data.lastLaunchTime + LAUNCH_COOLDOWN) - time;
            case CooldownSource::Damage_Hand:
                return (data.lastHandDamageTime + HANDDAMAGE_COOLDOWN) - time; 
            case CooldownSource::Damage_Thigh:
                return (data.lastThighDamageTime + THIGHDAMAGE_COOLDOWN) - time;
            case CooldownSource::Push_Basic:
                return (data.lastPushTime + PUSH_COOLDOWN) - time;
            case CooldownSource::Action_ButtCrush:
                return (data.lastButtCrushTime + Calculate_ButtCrushTimer(giant)) - time;
            case CooldownSource::Action_HealthGate:
                return (data.lastHealthGateTime + HEALTHGATE_COOLDOWN) - time;
            case CooldownSource::Action_ScareOther:   
                return time -(data.lastScareTime + SCARE_COOLDOWN) - time;
            case CooldownSource::Action_Hugs:
                return (data.lastHugTime + HUGS_COOLDOWN) - time;
            case CooldownSource::Action_AbsorbOther:
                return (data.lastAbsorbTime + Calculate_AbsorbCooldown(giant)) - time;    
            case CooldownSource::Action_Breasts_Suffocate:
                return (data.lastBreastSuffocateTime + Calculate_BreastActionCooldown(giant, 0)) - time;
            case CooldownSource::Action_Breasts_Vore:
                return (data.lastBreastVoreTime + Calculate_BreastActionCooldown(giant, 1)) - time;
            case CooldownSource::Action_Breasts_Absorb:
                return (data.lastBreastAbsorbTime + Calculate_BreastActionCooldown(giant, 2)) - time;
            case CooldownSource::Emotion_Laugh:   
                return (data.lastLaughTime + LAUGH_COOLDOWN) - time;
            case CooldownSource::Emotion_Moan: 
                return (data.lastMoanTime + MOAN_COOLDOWN) - time;
            case CooldownSource::Misc_RevertSound: 
                return (data.lastRevertTime + SOUND_COOLDOWN) - time;
            case CooldownSource::Misc_GrowthSound:
                return (data.lastSoundGrowthTime + GROW_SOUND_COOLDOWN) - time;
            case CooldownSource::Misc_BeingHit:
                return (data.lastHitTime + HIT_COOLDOWN) - time;  
            case CooldownSource::Misc_AiGrowth:
                return (data.lastGrowthTime + AI_GROWTH_COOLDOWN) - time;  
            case CooldownSource::Misc_ShrinkOutburst:
                return (data.lastOutburstTime + Calculate_ShrinkOutbirstTimer(giant)) - time; 
            case CooldownSource::Misc_ShrinkOutburst_Forced:
                return (data.lastForceOutburstTime + SHRINK_OUTBURST_COOLDOWN_FORCED) - time; 
            case CooldownSource::Footstep_Right:
                return (data.lastFootstepTime_R + Calculate_FootstepTimer(giant)) - time;   
            case CooldownSource::Footstep_Left:
                return (data.lastFootstepTime_L + Calculate_FootstepTimer(giant)) - time; 
            }
        return 0.0;
    }

    bool IsActionOnCooldown(Actor* giant, CooldownSource source) {
        float time = Time::WorldTimeElapsed();
        auto& data = CooldownManager::GetSingleton().GetCooldownData(giant);

        switch (source) {
            case CooldownSource::Damage_Launch: 
                return time <= (data.lastLaunchTime + LAUNCH_COOLDOWN);
            case CooldownSource::Damage_Hand:
                return time <= (data.lastHandDamageTime + HANDDAMAGE_COOLDOWN);
            case CooldownSource::Damage_Thigh:
                return time <= (data.lastThighDamageTime + THIGHDAMAGE_COOLDOWN);
            case CooldownSource::Push_Basic:
                return time <= (data.lastPushTime + PUSH_COOLDOWN);
            case CooldownSource::Action_ButtCrush:
                return time <= (data.lastButtCrushTime + Calculate_ButtCrushTimer(giant));
            case CooldownSource::Action_HealthGate:
                return time <= (data.lastHealthGateTime + HEALTHGATE_COOLDOWN);
            case CooldownSource::Action_ScareOther:   
                return time <= (data.lastScareTime + SCARE_COOLDOWN);
            case CooldownSource::Action_Breasts_Suffocate:
                return time <= (data.lastBreastSuffocateTime + Calculate_BreastActionCooldown(giant, 0));
            case CooldownSource::Action_Breasts_Vore:
                return time <= (data.lastBreastVoreTime + Calculate_BreastActionCooldown(giant, 1));
            case CooldownSource::Action_Breasts_Absorb:
                return time <= (data.lastBreastAbsorbTime + Calculate_BreastActionCooldown(giant, 2));
            case CooldownSource::Action_Hugs:
                return time <= (data.lastHugTime + HUGS_COOLDOWN);
            case CooldownSource::Action_AbsorbOther:
                return time <= (data.lastAbsorbTime + Calculate_AbsorbCooldown(giant));
            case CooldownSource::Emotion_Laugh:   
                return time <= (data.lastLaughTime + LAUGH_COOLDOWN);
            case CooldownSource::Emotion_Moan: 
                return time <= (data.lastMoanTime + MOAN_COOLDOWN);
            case CooldownSource::Misc_RevertSound: 
                return time <= (data.lastRevertTime + SOUND_COOLDOWN);
            case CooldownSource::Misc_GrowthSound:
                return time <= (data.lastSoundGrowthTime + GROW_SOUND_COOLDOWN);
            case CooldownSource::Misc_BeingHit:
                return time <= (data.lastHitTime + HIT_COOLDOWN);  
            case CooldownSource::Misc_AiGrowth:
                return time <= (data.lastGrowthTime + AI_GROWTH_COOLDOWN);
            case CooldownSource::Misc_ShrinkOutburst:
                return time <= (data.lastOutburstTime + Calculate_ShrinkOutbirstTimer(giant));   
            case CooldownSource::Misc_ShrinkOutburst_Forced:
                return time <= (data.lastForceOutburstTime + SHRINK_OUTBURST_COOLDOWN_FORCED);   
            case CooldownSource::Footstep_Right:
                return time <= (data.lastFootstepTime_R + Calculate_FootstepTimer(giant));    
            case CooldownSource::Footstep_Left:
                return time <= (data.lastFootstepTime_L + Calculate_FootstepTimer(giant));  
            }
        return false; 
    }
}