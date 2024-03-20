#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/Rumble.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
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
	const double LAUNCH_COOLDOWN = 0.8f;
	const double PUSH_COOLDOWN = 2.0f;
	const double HANDDAMAGE_COOLDOWN = 0.6f;
	const double THIGHDAMAGE_COOLDOWN = 1.2f;

	const double HEALTHGATE_COOLDOWN = 60.0f;
	const double SCARE_COOLDOWN = 6.0f;
	const double BUTTCRUSH_COOLDOWN = 30.0f;
	const double HUGS_COOLDOWN = 8.0f;

    const double LAUGH_COOLDOWN = 5.0f;
	const double MOAN_COOLDOWN = 5.0f;


    float Calculate_ButtCrushTimer(Actor* actor) {
		bool lvl70 = Runtime::HasPerk(actor, "ButtCrush_UnstableGrowth");
		bool lvl100 = Runtime::HasPerk(actor, "ButtCrush_LoomingDoom");
		float reduction = 0.0;
		if (lvl100) { // 25% reduction
			reduction += 7.5;
		} else if (lvl70) { // 15% reduction
			reduction += 4.5;
		} 
		return reduction;
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
            case CooldownSource::Emotion_Laugh:   
                data.lastLaughTime = Time::WorldTimeElapsed();
                break; 
            case CooldownSource::Emotion_Moan: 
                data.lastMoanTime = Time::WorldTimeElapsed();
                break;         
        }
    }

    bool IsActionOnCooldown(Actor* giant, CooldownSource source) {
        float time = Time::WorldTimeElapsed();
        auto& data = CooldownManager::GetSingleton().GetCooldownData(giant);

        switch (source) {
            case CooldownSource::Damage_Launch: 
                return time <= (data.lastLaunchTime + LAUNCH_COOLDOWN);
                break;
            case CooldownSource::Damage_Hand:
                return time <= (data.lastHandDamageTime + HANDDAMAGE_COOLDOWN);
                break;    
            case CooldownSource::Damage_Thigh:
                return time <= (data.lastThighDamageTime + THIGHDAMAGE_COOLDOWN);
                break;
            case CooldownSource::Push_Basic:
                return time <= (data.lastPushTime + PUSH_COOLDOWN);
                break;   
            case CooldownSource::Action_ButtCrush:
                return time <= (data.lastButtCrushTime + (BUTTCRUSH_COOLDOWN - Calculate_ButtCrushTimer(giant)));
                break;
            case CooldownSource::Action_HealthGate:
                return time <= (data.lastHealthGateTime + HEALTHGATE_COOLDOWN);
                break;
            case CooldownSource::Action_ScareOther:   
                return time <= (data.lastScareTime + SCARE_COOLDOWN);
                break; 
            case CooldownSource::Action_Hugs:
                return time <= (data.lastHugTime + HUGS_COOLDOWN);
                break;   
            }
        return false; 
    }
}