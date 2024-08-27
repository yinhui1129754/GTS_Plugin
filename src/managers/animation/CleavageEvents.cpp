#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/CleavageEvents.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;


namespace {
    const std::vector<std::string_view> BREAST_NODES = {
		"L Breast01",
		"R Breast01",
	};
    void ScaleBreasts(Actor* giant, float damage) {
        for (auto Nodes: BREAST_NODES) {
            auto node = find_node(giant, Nodes);
            if (node) {
                float limit = 1.50;
                if (node->local.scale < limit) {
                    node->local.scale += damage/100;
                } 
                if (node->local.scale >= limit) {
                    node->local.scale = limit;
                }
            }
        }
    }
    /// Functions
    void Deal_breast_damage(Actor* giant, float damage_mult) {
        Actor* tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            if (!IsTeammate(tiny)) {
                Attacked(tiny, giant); // force combat
            }

            if (IsHostile(giant, tiny)) {
                AnimationManager::StartAnim("Breasts_Idle_Unwilling", tiny);
            } else {
                AnimationManager::StartAnim("Breasts_Idle_Willing", tiny);
            }

            float bonus = 1.0;
            auto& sizemanager = SizeManager::GetSingleton();

			float tiny_scale = get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny);
			float gts_scale = get_visual_scale(giant) * GetSizeFromBoundingBox(giant);

			float sizeDiff = gts_scale/tiny_scale;
			float power = std::clamp(sizemanager.GetSizeAttribute(giant, SizeAttribute::Normal), 1.0f, 999999.0f);
			float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny);
			float damage = (Damage_Breast_Squish * damage_mult) * power * additionaldamage * additionaldamage * sizeDiff;
			float experience = std::clamp(damage/1600, 0.0f, 0.06f);

			if (HasSMT(giant)) {
				bonus = 1.65;
			}

            if (CanDoDamage(giant, tiny, false)) {
                if (Runtime::HasPerkTeam(giant, "GrowingPressure")) {
                    auto& sizemanager = SizeManager::GetSingleton();
                    sizemanager.ModSizeVulnerability(tiny, damage * 0.0010);
                }

                TinyCalamity_ShrinkActor(giant, tiny, damage * 0.20 * GetDamageSetting());

                SizeHitEffects::GetSingleton().BreakBones(giant, tiny, 0.15, 6);
                if (!IsTeammate(tiny)) {
                    InflictSizeDamage(giant, tiny, damage);
                } else {
                    tiny->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage * 5);
                }

                DamageAV(giant, ActorValue::kHealth, -damage * 0.5);
            }
			
			Rumbling::Once("GrabAttack", tiny, Rumble_Grab_Hand_Attack * bonus * damage_mult, 0.05, "NPC Root [Root]", 0.0);

            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, 1.0, 1.0, "NPC Root [Root]");

			ModSizeExperience(giant, experience);

            Utils_CrushTask(giant, tiny, bonus, false, false, DamageSource::BreastImpact, QuestStage::Crushing);
        }
    }
    /// Camera
    void GTS_BS_CamOn(const AnimationEventData& data) {ManageCamera(&data.giant, true, CameraTracking::ObjectB);}
    void GTS_BS_CamOff(const AnimationEventData& data) {ManageCamera(&data.giant, false, CameraTracking::ObjectB);}

    /// Attacks
    void GTS_BS_DamageTiny_L(const AnimationEventData& data) {
        Deal_breast_damage(&data.giant, 1.0);
    }
    void GTS_BS_DamageTiny_H(const AnimationEventData& data) {
        Deal_breast_damage(&data.giant, 2.25);
    }

    void GTS_BS_Shake(const AnimationEventData& data) {
        Actor* giant = &data.giant;
        auto tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            float scale_limit = 0.035;
            if (get_target_scale(tiny) > scale_limit) {
                set_target_scale(tiny, get_target_scale(tiny) * 0.975);
                DamageAV(giant, ActorValue::kHealth, -get_target_scale(tiny) * 10);
            } else {
                set_target_scale(tiny, scale_limit);
            }
            
            if (!IsActionOnCooldown(giant, CooldownSource::Emotion_Laugh)) {
                Task_FacialEmotionTask_Smile(giant, 6.0 / AnimationManager::GetAnimSpeed(giant), "ShakeSmile");
                ApplyActionCooldown(giant, CooldownSource::Emotion_Laugh);
            }

            NiPoint3 Position = GetHeartPosition(giant, tiny);
            Position.z -= 35 * get_visual_scale(giant);
            SpawnCustomParticle(giant, ParticleType::Hearts, Position, "NPC COM [COM ]", get_visual_scale(giant) * 0.5);
            DamageAV(tiny, ActorValue::kStamina, 18);
        }
    }

    /// Vore
    void GTS_BS_Smile(const AnimationEventData& data) {
    }
    void GTS_BS_OpenMouth(const AnimationEventData& data) {
        auto giant = &data.giant;
		auto tiny = Grab::GetHeldActor(giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		if (tiny) {
			SetBeingEaten(tiny, true);
			Vore::GetSingleton().ShrinkOverTime(giant, tiny, 0.1);
		}
		Task_FacialEmotionTask_OpenMouth(giant, 0.80 / AnimationManager::GetAnimSpeed(giant), "PrepareVore");
    }
    void GTS_BS_CloseMouth(const AnimationEventData& data) {
		/*AdjustFacialExpression(&data.giant, 0, 0.0, "phenome"); // Close mouth
		AdjustFacialExpression(&data.giant, 1, 0.0, "phenome"); // Close it

		AdjustFacialExpression(&data.giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(&data.giant, 1, 0.0, "modifier"); // blink R*/
    }
    void GTS_BS_PrepareEat(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		if (tiny) {
			VoreData.AddTiny(tiny);
		}
    }
    void GTS_BS_Swallow(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		if (tiny) {
            Runtime::PlaySoundAtNode("VoreSwallow", &data.giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
			for (auto& tiny: VoreData.GetVories()) {
				if (!AllowDevourment()) {
					VoreData.Swallow();
					if (IsCrawling(&data.giant)) {
						tiny->SetAlpha(0.0); // Hide Actor
					}
				} else {
					CallDevourment(&data.giant, tiny);
				}
			}
		}
    }
    void GTS_BS_KillAll(const AnimationEventData& data) {
        auto giant = &data.giant;
		auto tiny = Grab::GetHeldActor(&data.giant);
		if (tiny) {
			SetBeingEaten(tiny, false);
			auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
			for (auto& tiny: VoreData.GetVories()) {
				VoreData.KillAll();
			}
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
            //SpawnCustomParticle(giant, ParticleType::Hearts, NiPoint3, "NPC COM [COM ]", 1.5);
			//AnimationManager::StartAnim("TinyDied", giant);
			//BlockFirstPerson(giant, false);
			//ManageCamera(&data.giant, false, CameraTracking::Grab_Left);
			SetBeingHeld(tiny, false);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}
    }

    /// Absorb
    void GTS_BS_AbsorbStart(const AnimationEventData& data) {}
    void GTS_BS_AbsorbPulse(const AnimationEventData& data) {}
    void GTS_BS_FinishAbsorb(const AnimationEventData& data) {}
    void GTS_BS_GrowBoobs(const AnimationEventData& data) {}

    /// Utils
    void GTS_BS_SwitchToObjectB(const AnimationEventData& data) {Attachment_SetTargetNode(&data.giant, AttachToNode::ObjectB);}
    void GTS_BS_SwitchToCleavage(const AnimationEventData& data) {Attachment_SetTargetNode(&data.giant, AttachToNode::None);}
    void GTS_BS_Pat(const AnimationEventData& data) {}
}

namespace Gts
{
	void Animation_CleavageEvents::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_BS_CamOn", "Cleavage", GTS_BS_CamOn);
        AnimationManager::RegisterEvent("GTS_BS_CamOff", "Cleavage", GTS_BS_CamOff);

        AnimationManager::RegisterEvent("GTS_BS_DamageTiny_L", "Cleavage", GTS_BS_DamageTiny_L);
        AnimationManager::RegisterEvent("GTS_BS_DamageTiny_H", "Cleavage", GTS_BS_DamageTiny_H);
        AnimationManager::RegisterEvent("GTS_BS_Shake", "Cleavage", GTS_BS_Shake);

        AnimationManager::RegisterEvent("GTS_BS_Smile", "Cleavage", GTS_BS_Smile);
        AnimationManager::RegisterEvent("GTS_BS_OpenMouth", "Cleavage", GTS_BS_OpenMouth);
        AnimationManager::RegisterEvent("GTS_BS_CloseMouth", "Cleavage", GTS_BS_CloseMouth);

        AnimationManager::RegisterEvent("GTS_BS_PrepareEat", "Cleavage", GTS_BS_PrepareEat);
        AnimationManager::RegisterEvent("GTS_BS_Swallow", "Cleavage", GTS_BS_Swallow);
        AnimationManager::RegisterEvent("GTS_BS_KillAll", "Cleavage", GTS_BS_KillAll);

        AnimationManager::RegisterEvent("GTS_BS_AbsorbStart", "Cleavage", GTS_BS_AbsorbStart);
        AnimationManager::RegisterEvent("GTS_BS_AbsorbPulse", "Cleavage", GTS_BS_AbsorbPulse);
        AnimationManager::RegisterEvent("GTS_BS_FinishAbsorb", "Cleavage", GTS_BS_FinishAbsorb);
        AnimationManager::RegisterEvent("GTS_BS_GrowBoobs", "Cleavage", GTS_BS_GrowBoobs);

        AnimationManager::RegisterEvent("GTS_BS_SwitchToObjectB", "Cleavage", GTS_BS_SwitchToObjectB);
        AnimationManager::RegisterEvent("GTS_BS_SwitchToCleavage", "Cleavage", GTS_BS_SwitchToCleavage);

        AnimationManager::RegisterEvent("GTS_BS_Shake", "Cleavage", GTS_BS_Shake);




        AnimationManager::RegisterEvent("GTS_BS_Pat", "Cleavage", GTS_BS_Pat);
	}
} 
 
 
 