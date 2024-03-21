#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/HugHeal.hpp"
#include "colliders/charcontroller.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"


#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;


namespace {
	void ActivateEmotions(Actor* actor, bool toggle) {
		float p_1 = 1.0;
		float p_2 = 0.75;
		int rng = rand()% 8;
		if (!toggle) {
			p_1 = 0.0;
			p_2 = 0.0;
		} if (rng <= 1) {
			PlayMoanSound(actor, 1.0);
		}
		AdjustFacialExpression(actor, 0, p_1, "modifier"); // blink L
		AdjustFacialExpression(actor, 1, p_1, "modifier"); // blink R
		AdjustFacialExpression(actor, 0, p_2, "phenome");
	}

	void AbortHugAnimation_Friendly(Actor* giant) {
		auto tiny = HugShrink::GetHuggiesActor(giant);
		if (tiny) {
			EnableCollisions(tiny);
			SetBeingHeld(tiny, false);
			UpdateFriendlyHugs(giant, tiny, true); // set GTS_IsFollower (tiny) and GTS_HuggingTeammate (GTS) bools to false
			Hugs_FixAnimationDesync(giant, tiny, true); // reset anim speed override so .dll won't use it
		}
		log::info("Calling Abort Hug_Friendly");
		
		std::string name_normal = std::format("Huggies_{}", giant->formID);
		std::string name_forced = std::format("Huggies_Forced_{}", giant->formID);

		TaskManager::Cancel(name_normal);
		TaskManager::Cancel(name_forced);
		HugShrink::Release(giant);

		ControlAnother(giant, true);
	}

    bool Hugs_RestoreHealth(Actor* giantref, Actor* tinyref) {
		static Timer HeartTimer = Timer(0.5);
		float hp = GetAV(tinyref, ActorValue::kHealth);
		float maxhp = GetMaxAV(tinyref, ActorValue::kHealth);
		bool Healing = IsHugHealing(giantref);
		
		if (Healing && HeartTimer.ShouldRunFrame()) {
			NiPoint3 POS = GetHeartPosition(giantref, tinyref);
			if (POS.Length() > 0) {
				float scale = get_visual_scale(giantref);
				SpawnParticle(giantref, 3.00, "GTS/Magic/Hearts.nif", NiMatrix3(), POS, scale * 2.4, 7, nullptr);
			}
		} 

		if (!Healing && hp >= maxhp) {
			AbortHugAnimation(giantref, tinyref);
			if (giantref->formID == 0x14) {
				Notify("{} health is full", tinyref->GetDisplayFullName());
			}
			return false;
		} 

		if (giantref->formID == 0x14) {
			float sizedifference = get_visual_scale(giantref)/get_visual_scale(tinyref);
			shake_camera(giantref, 0.30 * sizedifference, 0.05);
		} else {
			GRumble::Once("HugSteal", giantref, 3.25, 0.10);
		}
		tinyref->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, maxhp * 0.004 * 0.15 * TimeScale());

		if (!Healing) {
			return false;
		}

		return true;
	}

    void HealOtherTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("Huggies_Heal_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			float sizedifference = GetSizeDifference(giantref, tinyref, false, true);
			float threshold = 3.0;
			float stamina = 0.35;

			if (Runtime::HasPerkTeam(giantref, "HugCrush_Greed")) {
				stamina *= 0.75;
			}
			stamina *= Perk_GetCostReduction(giantref);

			if (sizedifference >= threshold || sizedifference < Action_Hug) {
				SetBeingHeld(tinyref, false);
				AbortHugAnimation(giantref, tinyref);
				if (giantref->formID == 0x14) {
					shake_camera(giantref, 0.50, 0.15);
					Notify("It's difficult to gently hug {}", tinyref->GetDisplayFullName());
				}
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, -(0.45 * TimeScale())); // Restore Tiny stamina
			DamageAV(giantref, ActorValue::kStamina, 0.25 * stamina * TimeScale()); // Damage GTS Stamina

			return Hugs_RestoreHealth(giantref, tinyref);

		});
	}

    void GTS_Hug_Heal(AnimationEventData& data) {
        auto huggedActor = HugShrink::GetHuggiesActor(&data.giant);
		if (huggedActor) {
			HealOtherTask(&data.giant, huggedActor);
		}
    }

	void GTS_Hug_Release(AnimationEventData& data) {AbortHugAnimation_Friendly(&data.giant);}

	void GTS_Hug_Moan_Tiny(AnimationEventData& data) {ActivateEmotions(&data.giant, true);}
	void GTS_Hug_Moan_Tiny_End(AnimationEventData& data) {ActivateEmotions(&data.giant, false);}
}


namespace Gts {
    void HugHeal::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_Hug_Heal", "Hugs", GTS_Hug_Heal);
		AnimationManager::RegisterEvent("GTS_Hug_Release", "Hugs", GTS_Hug_Release);
		AnimationManager::RegisterEvent("GTS_Hug_Moan_Tiny", "Hugs", GTS_Hug_Moan_Tiny);
		AnimationManager::RegisterEvent("GTS_Hug_Moan_Tiny_End", "Hugs", GTS_Hug_Moan_Tiny_End);
    }

    void HugHeal::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Heal", "Hugs", "GTSBEH_HugHealStart_A");
		AnimationManager::RegisterTrigger("Huggies_Heal_Victim_F", "Hugs", "GTSBEH_HugHealStart_Fem_V");
		AnimationManager::RegisterTrigger("Huggies_Heal_Victim_M", "Hugs", "GTSBEH_HugHealStart_Mal_V");
	}
}