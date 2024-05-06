#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "utils/looting.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"


#include "profiler.hpp"

#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "NPC L Toe0 [LToe]";
	const std::string_view rightToeLookup = "NPC R Toe0 [RToe]";

    void ScareEnemies(Actor* giant)  {
		int FearChance = rand() % 2;
		if (FearChance <= 0) {
			Runtime::CastSpell(giant, giant, "GtsVoreFearSpell");
		}
	}

    void PlayGoreEffects(Actor* giant, Actor* tiny) {
        if (!IsLiving(tiny)) {
            SpawnDustParticle(tiny, giant, "NPC Root [Root]", 3.0);
        } else {
            if (!LessGore()) {
                auto root = find_node(tiny, "NPC Root [Root]");
                if (root) {
                    float currentSize = get_visual_scale(tiny);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, currentSize * 12.5, 7, root);
                }
            }
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::CreateExplosion(tiny, get_visual_scale(tiny) * 0.5, "BloodExplosion");
        }
    }

    void MoveItems(ActorHandle giantHandle, ActorHandle tinyHandle, FormID ID) {
        std::string taskname = std::format("CollisionDeath {}", ID);
        TaskManager::RunOnce(taskname, [=](auto& update){
            if (!tinyHandle) {
                return;
            }
            if (!giantHandle) {
                return;
            }
            auto giant = giantHandle.get().get();
            auto tiny = tinyHandle.get().get();
            float scale = get_visual_scale(tiny);
            TransferInventory(tiny, giant, scale, false, true, DamageSource::Collision, true);
        });
    }

    void RefreshDuration(Actor* giant) {
        if (Runtime::HasPerk(giant, "NoSpeedLoss")) {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.75); // Reduce speed after crush
        } else {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.35); // Reduce more speed after crush
        }
    }

    bool Collision_AllowTinyCalamityCrush(Actor* giant, Actor* tiny) {
        if (IsEssential(giant, tiny)) {
            return false;
        }
        float giantHp = GetAV(giant, ActorValue::kHealth);
		float tinyHp = GetAV(tiny, ActorValue::kHealth);

        float Multiplier = (get_visual_scale(giant) + 0.5) / get_visual_scale(tiny);

        if (giantHp >= ((tinyHp / Multiplier) * 1.25)) {
            return true;
        } else {
            return false;
        }
    }

    void FullSpeed_ApplyEffect(Actor* giant, float speed) {
        auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
            bool& CanApplyEffect = transient->SMT_ReachedFullSpeed;
            if (speed < 1.0) {
                CanApplyEffect = true;
            } else if (speed >= 1.0 && CanApplyEffect) {
                CanApplyEffect = false;
                //Runtime::PlaySoundAtNode("TinyCalamity_ReachedSpeed", giant, 1.0, 1.0, "NPC COM [COM ]");
            } 
        }
    }
}

namespace Gts {
    void TinyCalamity_ShrinkActor(Actor* giant, Actor* tiny, float shrink) {
        auto profiler = Profilers::Profile("Calamity: Shrink");
        if (HasSMT(giant)) {
            bool HasPerk = Runtime::HasPerk(giant, "SmallMassiveThreatSizeSteal");
            float limit = Minimum_Actor_Scale;
            if (HasPerk) {
				giant->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, shrink * 0.12);
                shrink *= 1.25;
			}

            float target_scale = get_target_scale(tiny);

            if (target_scale > limit/GetSizeFromBoundingBox(tiny)) {
                if ((target_scale - shrink*0.0045) <= limit || target_scale <= limit) {
                    set_target_scale(tiny, limit);
                    return;
                }
                ShrinkActor(tiny, shrink * 0.0045, 0.0);
            } else { // cap it just in case
                set_target_scale(tiny, limit);
            }
        }
    }

    void TinyCalamity_ExplodeActor(Actor* giant, Actor* tiny) {
        ModSizeExperience_Crush(giant, tiny, true);

        if (!tiny->IsDead()) {
            KillActor(giant, tiny);
        }

        ActorHandle giantHandle = giant->CreateRefHandle();
        ActorHandle tinyHandle = tiny->CreateRefHandle();

        CrushBonuses(giant, tiny);                             // common.hpp
        PlayGoreEffects(tiny, giant);    
        MoveItems(giantHandle, tinyHandle, tiny->formID);

        Attacked(tiny, giant);
        
        PrintDeathSource(giant, tiny, DamageSource::Collision);

        float OldScale;
        giant->GetGraphVariableFloat("GiantessScale", OldScale); // save old scale
        giant->SetGraphVariableFloat("GiantessScale", 1.0); // Needed to allow Stagger to play, else it won't work

        shake_camera_at_node(giant, "NPC COM [COM ]", 24.0, 1.0);
        StaggerActor(giant, 0.5f);
        RefreshDuration(giant);

        Runtime::PlaySound("GtsCrushSound", giant, 1.0, 1.0);

        if (tiny->formID != 0x14) {
            Disintegrate(tiny, true); // Set critical stage 4 on actors
        } else if (tiny->formID == 0x14) {
            TriggerScreenBlood(50);
            tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
        }

        giant->SetGraphVariableFloat("GiantessScale", OldScale);
        Runtime::PlaySoundAtNode("TinyCalamity_Crush", giant, 1.0, 1.0, "NPC COM [COM ]");

        ScareEnemies(giant);
    }

    void TinyCalamity_StaggerActor(Actor* giant, Actor* tiny, float giantHp) { // when we can't crush the target
        float OldScale; 
        giant->GetGraphVariableFloat("GiantessScale", OldScale); // record old slace
        giant->SetGraphVariableFloat("GiantessScale", 1.0); // Needed to allow Stagger to play, else it won't work

        PushForward(giant, tiny, 800);
        AddSMTDuration(giant, 2.5);
        StaggerActor(giant, 0.5); // play stagger on the player

        Attacked(tiny, giant);

        DamageAV(tiny, ActorValue::kHealth, giantHp * 0.75);
        DamageAV(giant, ActorValue::kHealth, giantHp * 0.25);

        float hpcalc = (giantHp * 0.75f)/800.0;
        float xp = std::clamp(hpcalc, 0.0f, 0.12f);
        update_target_scale(tiny, -0.06, SizeEffectType::kShrink);
        ModSizeExperience(giant, xp);

        Runtime::PlaySoundAtNode("TinyCalamity_Impact", giant, 1.0, 1.0, "NPC COM [COM ]");
        shake_camera_at_node(giant, "NPC COM [COM ]", 16.0, 1.0);
        
        if (IsEssential(giant, tiny)) {
            Notify("{} is essential", tiny->GetDisplayFullName());
        } else {
            Notify("{} is too tough to be crushed", tiny->GetDisplayFullName());
        }

        giant->SetGraphVariableFloat("GiantessScale", OldScale);
        RefreshDuration(giant);
    }

    void TinyCalamity_SeekActors(Actor* giant) {
        auto profiler = Profilers::Profile("Calamity: SeekActor");
        if (giant->formID == 0x14) {
            if (giant->AsActorState()->IsSprinting() && HasSMT(giant)) {
                auto node = find_node(giant, "NPC Pelvis [Pelv]");
                if (!node) {
                    return;
                }
                NiPoint3 NodePosition = node->world.translate;

                float giantScale = get_visual_scale(giant);

                const float BASE_DISTANCE = 48.0;
                float CheckDistance = BASE_DISTANCE*giantScale;

                if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
                    DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 100, {0.0, 1.0, 1.0, 1.0});
                }

                NiPoint3 giantLocation = giant->GetPosition();
                for (auto otherActor: find_actors()) {
                    if (otherActor != giant) {
                        NiPoint3 actorLocation = otherActor->GetPosition();
                        if ((actorLocation - giantLocation).Length() < BASE_DISTANCE*giantScale*3) {
                            int nodeCollisions = 0;
                            float force = 0.0;

                            auto model = otherActor->GetCurrent3D();

                            if (model) {
                                VisitNodes(model, [&nodeCollisions, &force, NodePosition, CheckDistance](NiAVObject& a_obj) {
                                    float distance = (NodePosition - a_obj.world.translate).Length();
                                    if (distance < CheckDistance) {
                                        nodeCollisions += 1;
                                        force = 1.0 - distance / CheckDistance;
                                        return false;
                                    }
                                    return true;
                                });
                            }
                            if (nodeCollisions > 0) {
                                TinyCalamity_CrushCheck(giant, otherActor);
                            }
                        }
                    }
                }
            }
        }
    }

    void TinyCalamity_CrushCheck(Actor* giant, Actor* tiny) {
		auto profiler = Profilers::Profile("tinyCalamity: CrushCheck");
		if (giant == tiny) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(giant)) {
			if (persistent.GetData(giant)->smt_run_speed >= 1.0) {
                float giantHp = GetAV(giant, ActorValue::kHealth);

				if (giantHp <= 0) {
					return; // just in case, to avoid CTD
				}

				if (Collision_AllowTinyCalamityCrush(giant, tiny)) {
                    StartCombat(tiny, giant);
                    TinyCalamity_ExplodeActor(giant, tiny);
				} else {
                    StartCombat(tiny, giant);
                    TinyCalamity_StaggerActor(giant, tiny, giantHp);
				}
			}
		}
	}

    void TinyCalamity_BonusSpeed(Actor* giant) { // Manages SMT bonus speed
		// Andy's TODO: Calc on demand rather than poll

		auto Attributes = Persistent::GetSingleton().GetData(giant);
		float Gigantism = 1.0 + (Ench_Aspect_GetPower(giant) * 0.25);

        float speed = 1.0; 
        float decay = 1.0;
        float cap = 1.0;

		float& currentspeed = Attributes->smt_run_speed;
		if (giant->AsActorState()->IsSprinting() && HasSMT(giant)) { // SMT Active and sprinting
			if (Runtime::HasPerk(giant, "NoSpeedLoss")) {
				speed = 1.25;
                decay = 1.5;
				cap = 1.25;
			}

			currentspeed += 0.004400 * speed * Gigantism * 5; // increase MS

			if (currentspeed > cap) {
				currentspeed = cap;
			}

            FullSpeed_ApplyEffect(giant, currentspeed);
		} else { // else decay bonus speed over time
			if (currentspeed > 0.0) {
				currentspeed -= (0.045000 * 5) / decay;
			} else if (currentspeed <= 0.0) {
				currentspeed = 0.0;
			} 
		}
    }
}