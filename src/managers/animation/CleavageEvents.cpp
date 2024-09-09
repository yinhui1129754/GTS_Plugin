#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/CleavageEvents.hpp"
#include "managers/animation/CleavageState.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "utils/DifficultyUtils.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "utils/looting.hpp"
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
    void CancelAnimation(Actor* giant) {
        auto tiny = Grab::GetHeldActor(giant);
        giant->SetGraphVariableBool("GTS_OverrideZ", false);
        if (tiny) {
            KillActor(giant, tiny, false);
            DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75);
            tiny->SetGraphVariableBool("GTSBEH_T_InStorage", false);
            SetBetweenBreasts(tiny, false);
            SetBeingEaten(tiny, false);
            SetBeingHeld(tiny, false);
        }

        std::string name = std::format("GrabAttach_{}", giant->formID);
        TaskManager::Cancel(name);

        Grab::DetachActorTask(giant);
		Grab::Release(giant);
    }

    void RecoverAttributes(Actor* giant, ActorValue Attribute, float percentage) {
        float Percent = GetMaxAV(giant, Attribute);
        float value = Percent * percentage;

        if (Runtime::HasPerk(giant, "Breasts_MasteryPart2")) {
            value *= 1.5;
        }

        DamageAV(giant, Attribute, -value);
    }

    void ShrinkTinyWithCleavage(Actor* giant, float scale_limit, float shrink_for, float stamina_damage, bool hearts, bool damage_stamina) {
        Actor* tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            if (get_target_scale(tiny) > scale_limit && shrink_for < 1.0) {
                DamageAV(giant, ActorValue::kHealth, -get_target_scale(tiny) * 10); // Heal GTS
                set_target_scale(tiny, get_target_scale(tiny) * shrink_for);
            } else {
                set_target_scale(tiny, scale_limit);
            }
            
            if (damage_stamina) {
                DamageAV(tiny, ActorValue::kStamina, stamina_damage);
            }
            if (hearts) {
                SpawnHearts(giant, tiny, 35, 0.5);
            }
        }
    }
    void SuffocateTinyFor(Actor* giant, Actor* tiny, float DamageMult, float Shrink, float StaminaDrain) {
        float TotalHP = GetMaxAV(tiny, ActorValue::kHealth);
        float CurrentHP = GetAV(tiny, ActorValue::kHealth);
        float Threshold = 1.0;
        
        float damage = TotalHP * DamageMult; // 100% hp by default
        
        if (CurrentHP > Threshold) {
            if (DamageMult > 0 && CurrentHP - damage > Threshold) {
                DamageAV(tiny, ActorValue::kHealth, damage);
            } else {
                if (DamageMult > 0) {
                    SetAV(tiny, ActorValue::kHealth, 1.0);
                }
            }
            DamageAV(tiny, ActorValue::kStamina, StaminaDrain);
        }
    }

    void Task_RunSuffocateTask(Actor* giant, Actor* tiny) {
        std::string name = std::format("SuffoTask_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();

        float starting_hppercentage = GetHealthPercentage(tiny);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			if (!tinyref) {
				return false; // end task in that case
			}
            float damage = 0.0;//0.0015 * starting_hppercentage * TimeScale();
            SuffocateTinyFor(giantref, tinyref, damage, 0.998, 0.0035 * TimeScale());

            if (tinyref->IsDead() || GetAV(tinyref, ActorValue::kHealth) <= 0) {
                return false;
            }
			// All good try another frame
			return true;
		});
    }

    void PrintBreastAbsorbed(Actor* giant, Actor* tiny) {
        int random = rand() % 5;
        if (random <= 1) {
            Cprint("{} suddenly disappeared between the breasts of {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
        } else if (random == 2) {
            Cprint("Mountains of {} greedily absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
        } else if (random == 3) {
            Cprint("{} became one with the breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
        } else {
            Cprint("{} was gently devoured by the milkers of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
        }
    }

    ///=================================================================== Functions
    void Deal_breast_damage(Actor* giant, float damage_mult) {
        Actor* tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            if (!IsTeammate(tiny)) {
                Attacked(tiny, giant); // force combat
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
                    DamageAV(tiny, ActorValue::kStamina, damage * 0.25);
                } else {
                    tiny->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage * 5);
                }

                DamageAV(giant, ActorValue::kHealth, -damage * 0.33);
            }
			
			Rumbling::Once("GrabAttack", tiny, Rumble_Grab_Hand_Attack * bonus * damage_mult, 0.05, "NPC Root [Root]", 0.0);
            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, 1.0, 1.0, "NPC Root [Root]");

            Utils_CrushTask(giant, tiny, bonus, false, false, DamageSource::BreastImpact, QuestStage::Crushing);
            ModSizeExperience(giant, experience);
        }
    }

    ///===================================================================

    ///=================================================================== Camera

    void GTS_BS_CamOn(const AnimationEventData& data) {ManageCamera(&data.giant, true, CameraTracking::Breasts_02);}
    void GTS_BS_CamOff(const AnimationEventData& data) {ManageCamera(&data.giant, false, CameraTracking::Breasts_02);}

    ///===================================================================

    ///=================================================================== Attacks

    void GTS_BS_SpringUp(const AnimationEventData& data) {
        ManageCamera(&data.giant, true, CameraTracking::ObjectB);
    }

    void GTS_BS_BoobsRelease(const AnimationEventData& data) {
    }

    void GTS_BS_DamageTiny_L(const AnimationEventData& data) {
        DamageAV(&data.giant, ActorValue::kStamina, 30 * GetWasteMult(&data.giant));

        Rumbling::Once("HandImpact_L", &data.giant, 0.3, 0.0, "L Breast02", 0.0);
        Rumbling::Once("HandImpact_R", &data.giant, 0.3, 0.0, "R Breast02", 0.0);

        Deal_breast_damage(&data.giant, 1.0);
    }
    void GTS_BS_DamageTiny_H(const AnimationEventData& data) {
        DamageAV(&data.giant, ActorValue::kStamina, 45 * GetWasteMult(&data.giant));

        Rumbling::Once("HandImpactH_L", &data.giant, 0.55, 0.0, "L Breast02", 0.0);
        Rumbling::Once("HandImpactH_R", &data.giant, 0.55, 0.0, "R Breast02", 0.0);

        Deal_breast_damage(&data.giant, 2.25);
    }

    void GTS_BS_Shake(const AnimationEventData& data) {
        ShrinkTinyWithCleavage(&data.giant, 0.035, 0.980, 25.0, true, true);

        Rumbling::Once("BreastShake_L", &data.giant, 0.3, 0.0, "L Breast02", 0.0);
        Rumbling::Once("BreastShake_R", &data.giant, 0.3, 0.0, "R Breast02", 0.0);

        if (!IsActionOnCooldown(&data.giant, CooldownSource::Emotion_Laugh)) {
            Task_FacialEmotionTask_Smile(&data.giant, 6.0 / AnimationManager::GetAnimSpeed(&data.giant), "ShakeSmile");
            ApplyActionCooldown(&data.giant, CooldownSource::Emotion_Laugh);
        }
    }

    ///===================================================================

    ///=================================================================== Vore

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
		Task_FacialEmotionTask_OpenMouth(giant, 0.66 / AnimationManager::GetAnimSpeed(giant), "PrepareVore");
    }
    void GTS_BS_CloseMouth(const AnimationEventData& data) {
    }
    void GTS_BS_PrepareEat(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        ManageCamera(&data.giant, true, CameraTracking::ObjectB);
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

        RecoverAttributes(&data.giant, ActorValue::kHealth, 0.07);
        RecoverAttributes(&data.giant, ActorValue::kMagicka, 0.07);
        RecoverAttributes(&data.giant, ActorValue::kStamina, 0.07);
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

			SetBeingHeld(tiny, false);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}

        Animation_Cleavage::LaunchCooldownFor(&data.giant, CooldownSource::Action_Breasts_Vore);
    }

    ///=================================================================== Absorb

    void GTS_BS_AbsorbStart(const AnimationEventData& data) {
        Task_FacialEmotionTask_Smile(&data.giant, 3.2 / AnimationManager::GetAnimSpeed(&data.giant), "AbsorbStart");
        Task_ApplyAbsorbCooldown(&data.giant);
    }

    void GTS_BS_AbsorbPulse(const AnimationEventData& data) {
        auto giant = &data.giant;
		auto tiny = Grab::GetHeldActor(&data.giant);
		if (tiny) {
            Rumbling::Once("AbsorbPulse_R", giant, 0.45, 0.0, "L Breast02", 0.0);
            Rumbling::Once("AbsorbPulse_L", giant, 0.45, 0.0, "R Breast02", 0.0);

            float volume = std::clamp(0.12f * get_visual_scale(giant), 0.12f, 1.0f);

            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, volume, 1.0, "NPC Root [Root]");

            bool Blocked = IsActionOnCooldown(giant, CooldownSource::Emotion_Laugh);
            if (!Blocked) {
                ApplyActionCooldown(giant, CooldownSource::Emotion_Laugh);
                PlayLaughSound(giant, 0.6, 1);
            }

            ShrinkTinyWithCleavage(giant, 0.010, 0.66, 45.0, true, true);
            RecoverAttributes(giant, ActorValue::kHealth, 0.025);
        }
    }

    void GTS_BS_FinishAbsorb(const AnimationEventData& data) {
        Actor* giant = &data.giant;
        Task_FacialEmotionTask_Moan(giant, 1.1 / AnimationManager::GetAnimSpeed(giant), "AbsorbMoan");
        auto tiny = Grab::GetHeldActor(giant);
		if (tiny) {
            SpawnHearts(giant, tiny, 35, 0.75);
            PlayMoanSound(giant, 0.8);

            AdvanceQuestProgression(giant, tiny, QuestStage::HugSteal, 1.0, false);

            Rumbling::Once("AbsorbTiny_R", giant, 0.8, 0.05, "L Breast02", 0.0);
            Rumbling::Once("AbsorbTiny_L", giant, 0.8, 0.05, "R Breast02", 0.0);

            DamageAV(giant, ActorValue::kHealth, -30); // Heal GTS
            PrintBreastAbsorbed(giant, tiny);
            AdjustSizeReserve(giant, 0.04);
            

            std::string taskname = std::format("MergeWithTiny{}", tiny->formID);
            ActorHandle giantHandle = giant->CreateRefHandle();
            ActorHandle tinyHandle = tiny->CreateRefHandle();
            TaskManager::RunOnce(taskname, [=](auto& update){
                if (!tinyHandle) {
                    return;
                }
                if (!giantHandle) {
                    return;
                }

                auto giant = giantHandle.get().get();
                auto tiny = tinyHandle.get().get();
                TransferInventory(tiny, giant, get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny), false, true, DamageSource::Vored, true);
                // Actor Reset is done inside TransferInventory:StartResetTask!
            });
            
            RecoverAttributes(giant, ActorValue::kHealth, 0.05);
            ModSizeExperience(giant, 0.235);
            Disintegrate(tiny, false);  
        }
    }
    void GTS_BS_GrowBoobs(const AnimationEventData& data) {
        Animation_Cleavage::LaunchCooldownFor(&data.giant, CooldownSource::Action_Breasts_Absorb);
    }

    ///===================================================================

    ///=================================================================== Suffocate

    void GTS_BS_SufoStart(const AnimationEventData& data) { 
        Actor* giant = &data.giant;
        auto tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            SuffocateTinyFor(&data.giant, tiny, 0.10, 0.85, 25.0);
            Task_RunSuffocateTask(giant, tiny);
            SpawnHearts(giant, tiny, 35, 0.35);
        }
        Task_FacialEmotionTask_Smile(&data.giant, 1.8 / AnimationManager::GetAnimSpeed(&data.giant), "SufoStart");
    }
    void GTS_BS_SufoStop(const AnimationEventData& data) {}

    void GTS_BS_SufoKill(const AnimationEventData& data) {
        Animation_Cleavage::LaunchCooldownFor(&data.giant, CooldownSource::Action_Breasts_Suffocate);
        ModSizeExperience(&data.giant, 0.235);
        CancelAnimation(&data.giant);
    }
    void GTS_BS_SufoPress(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            RecoverAttributes(&data.giant, ActorValue::kMagicka, 0.035);
            SuffocateTinyFor(&data.giant, tiny, 0.35, 0.85, 25.0);
            SpawnHearts(&data.giant, tiny, 35, 0.50);
        }
        Task_FacialEmotionTask_Smile(&data.giant, 1.2 / AnimationManager::GetAnimSpeed(&data.giant), "SufoPress");
    }
    
    void GTS_BS_PullTiny(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            Task_FacialEmotionTask_Smile(&data.giant, 1.6 / AnimationManager::GetAnimSpeed(&data.giant), "SufoPullOut");
            Rumbling::Once("PullOut_R", &data.giant, 0.75, 0.0, "L Breast02", 0.0);
            Rumbling::Once("PullOut_L", &data.giant, 0.75, 0.0, "R Breast02", 0.0);

            Attachment_SetTargetNode(&data.giant, AttachToNode::ObjectL);

            ManageCamera(&data.giant, true, CameraTracking::ObjectB);
            SpawnHearts(&data.giant, tiny, 35, 0.50);
        }
    }

    void GTS_BS_HandsLand(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            RecoverAttributes(&data.giant, ActorValue::kMagicka, 0.045);
            SuffocateTinyFor(&data.giant, tiny, 0.20, 0.75, 20.0);

            Rumbling::Once("HandLand_R", &data.giant, 0.45, 0.0, "L Breast02", 0.0);
            Rumbling::Once("HandLand_L", &data.giant, 0.45, 0.0, "R Breast02", 0.0);

            float volume = std::clamp(0.12f * get_visual_scale(&data.giant), 0.12f, 1.0f);
            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, volume, 1.0, "NPC Root [Root]");
        }
    }

    void GTS_BS_OverrideZ_ON(const AnimationEventData& data) { 
        data.giant.SetGraphVariableBool("GTS_OverrideZ", true);
    }
    void GTS_BS_OverrideZ_OFF(const AnimationEventData& data) { 
        data.giant.SetGraphVariableBool("GTS_OverrideZ", false);
    }

    ///===================================================================

    ///=================================================================== Utils

    void GTS_BS_SwitchToObjectB(const AnimationEventData& data) {Attachment_SetTargetNode(&data.giant, AttachToNode::ObjectB);}
    void GTS_BS_SwitchToCleavage(const AnimationEventData& data) {Attachment_SetTargetNode(&data.giant, AttachToNode::None);}

    void GTS_BS_Poke(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            SpawnHearts(&data.giant, tiny, 35, 0.4);
        }
    }
    void GTS_BS_Pat(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            SpawnHearts(&data.giant, tiny, 35, 0.4);

            if (IsHostile(&data.giant, tiny)) {
				AnimationManager::StartAnim("Breasts_Idle_Unwilling", tiny);
			} else {
				AnimationManager::StartAnim("Breasts_Idle_Willing", tiny);
			}
        }
    }

    ///===================================================================
}

namespace Gts
{
	void Animation_CleavageEvents::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_BS_CamOn", "Cleavage", GTS_BS_CamOn);
        AnimationManager::RegisterEvent("GTS_BS_CamOff", "Cleavage", GTS_BS_CamOff);

        AnimationManager::RegisterEvent("GTS_BS_SpringUp", "Cleavage", GTS_BS_SpringUp);
        AnimationManager::RegisterEvent("GTS_BS_BoobsRelease", "Cleavage", GTS_BS_BoobsRelease);
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

        AnimationManager::RegisterEvent("GTS_BS_SufoStart", "Cleavage", GTS_BS_SufoStart);
        AnimationManager::RegisterEvent("GTS_BS_SufoStop", "Cleavage", GTS_BS_SufoStop);
        AnimationManager::RegisterEvent("GTS_BS_SufoKill", "Cleavage", GTS_BS_SufoKill);
        AnimationManager::RegisterEvent("GTS_BS_SufoPress", "Cleavage", GTS_BS_SufoPress);
        AnimationManager::RegisterEvent("GTS_BS_PullTiny", "Cleavage", GTS_BS_PullTiny);
        AnimationManager::RegisterEvent("GTS_BS_OverrideZ_ON", "Cleavage", GTS_BS_OverrideZ_ON);
        AnimationManager::RegisterEvent("GTS_BS_OverrideZ_OFF", "Cleavage", GTS_BS_OverrideZ_OFF);

        AnimationManager::RegisterEvent("GTS_BS_SwitchToObjectB", "Cleavage", GTS_BS_SwitchToObjectB);
        AnimationManager::RegisterEvent("GTS_BS_SwitchToCleavage", "Cleavage", GTS_BS_SwitchToCleavage);

        AnimationManager::RegisterEvent("GTS_BS_Shake", "Cleavage", GTS_BS_Shake);
        AnimationManager::RegisterEvent("GTS_BS_HandsLand", "Cleavage", GTS_BS_HandsLand);
        AnimationManager::RegisterEvent("GTS_BS_Poke", "Cleavage", GTS_BS_Poke);
        AnimationManager::RegisterEvent("GTS_BS_Pat", "Cleavage", GTS_BS_Pat);
	}
} 
 
 
 