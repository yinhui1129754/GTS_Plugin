#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Growth.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	void SetHalfLife(Actor* actor, float value) {
		auto& Persist = Persistent::GetSingleton();
		auto actor_data = Persist.GetData(actor);
		if (actor_data) {
			actor_data->half_life = value;
		}
	}

	void CancelGrowth(Actor* actor) {
		std::string name = std::format("ManualGrowth_{}", actor->formID);
		TaskManager::Cancel(name);
		SetHalfLife(actor, 1.0);
	}

	void GrowthTask(Actor* actor) {
		if (!actor) {
			return;
		}
		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = actor->CreateRefHandle();
		std::string name = std::format("ManualGrowth_{}", actor->formID);

		float Volume = clamp(0.20, 1.0, get_visual_scale(actor)/8);
		Runtime::PlaySoundAtNode("growthSound", actor, Volume, 1.0, "NPC Pelvis [Pelv]");

		SetHalfLife(actor, 0.0);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto caster = gianthandle.get().get();
			float timepassed = Time::WorldTimeElapsed() - Start;
			float animspeed = AnimationManager::GetAnimSpeed(caster);
			float elapsed = std::clamp(timepassed * animspeed, 0.01f, 1.2f);
			float multiply = bezier_curve(elapsed, 0, 1.9, 0.6, 0, 2.0, 1.0);
			//                            ^value   x1  x2  x3  x4  i     k

			float caster_scale = get_visual_scale(caster);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

			float perk = Perk_GetCostReduction(caster);  
			
			DamageAV(caster, ActorValue::kStamina, 0.60 * perk * caster_scale * stamina * TimeScale() * multiply);
			Grow(caster, 0.0080 * stamina * multiply * animspeed, 0.0); // Is automatically *'d by scale through CalcPower()
			// value*scale ^  ; ^ static value, not affected by scale
			
			GRumble::Once("GrowButton", caster, 6.0 * stamina, 0.05, "NPC Pelvis [Pelv]");
			if (elapsed >= 0.99) {
				SetHalfLife(caster, 1.0);
				return false;
			}
			return true;
		});
	}
	void GtsGrowth_Moan(AnimationEventData& data) {
		PlayMoanSound(&data.giant, 1.0);
	}
	void GtsGrowth_Mouth_Open(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 1.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 1.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 0, 0.75, "phenome");
	}
	void GtsGrowth_Mouth_Close(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 0, 0.0, "phenome");
	}
	void GTSGrowth_Enter(AnimationEventData& data) {
	}
	void GTSGrowth_SpurtStart(AnimationEventData& data) {
		GrowthTask(&data.giant);
	}
	void GTSGrowth_SpurtSlowdownPoint(AnimationEventData& data) {
	}
	void GTSGrowth_SpurtStop(AnimationEventData& data) {
		CancelGrowth(&data.giant);
	}
	void GTSGrowth_Exit(AnimationEventData& data) {
	}
}

namespace Gts
{
	void AnimationGrowth::RegisterEvents() {
		AnimationManager::RegisterEvent("GtsGrowth_Moan", "Growth", GtsGrowth_Moan);
		AnimationManager::RegisterEvent("GtsGrowth_Mouth_Open", "Growth", GtsGrowth_Mouth_Open);
		AnimationManager::RegisterEvent("GtsGrowth_Mouth_Close", "Growth", GtsGrowth_Mouth_Close);

		AnimationManager::RegisterEvent("GTSGrowth_Enter", "Growth", GTSGrowth_Enter);
		AnimationManager::RegisterEvent("GTSGrowth_SpurtStart", "Growth", GTSGrowth_SpurtStart);
		AnimationManager::RegisterEvent("GTSGrowth_SpurtSlowdownPoint", "Growth", GTSGrowth_SpurtSlowdownPoint);
		AnimationManager::RegisterEvent("GTSGrowth_SpurtStop", "Growth", GTSGrowth_SpurtStop);
		AnimationManager::RegisterEvent("GTSGrowth_Exit", "Growth", GTSGrowth_Exit);
	}

	void AnimationGrowth::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TriggerGrowth", "Growth", "GTSBeh_Grow_Trigger");
	}
}