#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/Grab_Attack.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
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

    void Utils_CrushTask(Actor* giant, Actor* grabbedActor, float bonus) {
        
        auto tinyref = grabbedActor->CreateRefHandle();
        auto giantref = giant->CreateRefHandle();

        std::string taskname = std::format("GrabCrush_{}", grabbedActor->formID);

        TaskManager::RunOnce(taskname, [=](auto& update) {
            if (!tinyref) {
                return;
            } 
            if (!giantref) {
                return;
            }
            auto tiny = tinyref.get().get();
            auto giantess = giantref.get().get();

            if (GetAV(tiny, ActorValue::kHealth) <= 1.0 || tiny->IsDead()) {

                ModSizeExperience_Crush(giant, tiny, false);

                CrushManager::Crush(giantess, tiny);
                
                SetBeingHeld(tiny, false);
                GRumble::Once("GrabAttackKill", giantess, 14.0 * bonus, 0.15, "NPC L Hand [LHnd]");
                if (!LessGore()) {
                    Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
                    Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
                } else {
                    Runtime::PlaySoundAtNode("SoftHandAttack", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
                }
                Runtime::PlaySoundAtNode("GtsCrushSound", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
                AdjustSizeReserve(giantess, get_visual_scale(tiny)/10);
                SpawnHurtParticles(giantess, tiny, 3.0, 1.6);
                SpawnHurtParticles(giantess, tiny, 3.0, 1.6);
                
                SetBetweenBreasts(giantess, false);
                StartCombat(tiny, giantess);
                
                AdvanceQuestProgression(giantess, tiny, 5, 1.0, false);
                
                PrintDeathSource(giantess, tiny, DamageSource::HandCrushed);
                Grab::DetachActorTask(giantess);
                Grab::Release(giantess);
            } else {
                if (!LessGore()) {
                    Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
                    SpawnHurtParticles(giantess, tiny, 1.0, 1.0);
                } else {
                    Runtime::PlaySoundAtNode("SoftHandAttack", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
                }
                StaggerActor(giantess, tiny, 0.75f);
            }
        });
    }

    void GTSGrab_Attack_MoveStart(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabAttack", "DestructionBasics", true, 0.75);
		ManageCamera(giant, true, CameraTracking::Grab_Left);
		StartLHandRumble("GrabMoveL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Attack_Damage(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		float bonus = 1.0;
		auto giant = &data.giant;
		auto grabbedActor = Grab::GetHeldActor(giant);

		if (grabbedActor) {
			Attacked(grabbedActor, giant); // force combat

			float tiny_scale = get_visual_scale(grabbedActor) * GetSizeFromBoundingBox(grabbedActor);
			float gts_scale = get_visual_scale(giant) * GetSizeFromBoundingBox(giant);

			float sizeDiff = gts_scale/tiny_scale;
			float power = std::clamp(sizemanager.GetSizeAttribute(giant, 0), 1.0f, 999999.0f);
			float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(grabbedActor);
			float damage = (Damage_Grab_Attack * sizeDiff) * power * additionaldamage * additionaldamage;
			float experience = std::clamp(damage/800, 0.0f, 0.06f);
			if (HasSMT(giant)) {
				bonus = 3.0;
			}

			InflictSizeDamage(giant, grabbedActor, damage);

			GRumble::Once("GrabAttack", giant, 5.0 * bonus, 0.05, "NPC L Hand [LHnd]");

			SizeHitEffects::GetSingleton().BreakBones(giant, grabbedActor, 0, 1); // don't do damage and just add flat debuff
			SizeHitEffects::GetSingleton().BreakBones(giant, grabbedActor, 0, 1); // do it twice

			ModSizeExperience(giant, experience);
			AddSMTDuration(giant, 1.0);

            Utils_CrushTask(giant, grabbedActor, bonus);
		}
	}

	void GTSGrab_Attack_MoveStop(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& sizemanager = SizeManager::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(giant);
		ManageCamera(giant, false, CameraTracking::Grab_Left);
		DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75);
		StopLHandRumble("GrabMoveL", data.giant);
		if (!grabbedActor) {
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
			AnimationManager::StartAnim("GrabAbort", giant);
			AnimationManager::StartAnim("TinyDied", giant);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
			return;
		}
	}
}

namespace Gts {
    void Animation_GrabAttack::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStart", "Grabbing", GTSGrab_Attack_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Attack_Damage", "Grabbing", GTSGrab_Attack_Damage);
		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStop", "Grabbing", GTSGrab_Attack_MoveStop);
    }
}
