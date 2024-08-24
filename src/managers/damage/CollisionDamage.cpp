#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/audio/GoreAudio.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/MovementForce.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "managers/highheel.hpp"
#include "utils/DeathReport.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>


using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	bool StrongGore(DamageSource cause) {
		bool Strong = false;
		switch (cause) {
			case DamageSource::FootGrindedRight_Impact:
			case DamageSource::FootGrindedLeft_Impact:
			case DamageSource::RightFinger_Impact:
			case DamageSource::LeftFinger_Impact:
			case DamageSource::HandCrawlRight:
			case DamageSource::HandCrawlLeft:
			case DamageSource::KneeDropRight:
			case DamageSource::KneeDropLeft:
			case DamageSource::KneeRight:
			case DamageSource::KneeLeft:
			case DamageSource::HandDropRight:
			case DamageSource::HandDropLeft:
			case DamageSource::HandSlamRight:
			case DamageSource::HandSlamLeft:
			case DamageSource::CrushedRight:
			case DamageSource::CrushedLeft:
			case DamageSource::WalkRight:
			case DamageSource::WalkLeft:
			case DamageSource::BodyCrush:
			case DamageSource::BreastImpact:
			case DamageSource::Booty:
				Strong = true;
			break;
		}
		return Strong;
	}
	bool Allow_Damage(Actor* giant, Actor* tiny, DamageSource cause, float difference) {
		float threshold = 3.0;

		if (DisallowSizeDamage(giant, tiny)) {
			return false;
		}

		if (difference > threshold) {
			return true;
		}

		bool PreventDamage = false;

		switch (cause) {
			case DamageSource::WalkLeft:
			case DamageSource::WalkRight:
				PreventDamage = true;
			break;
			case DamageSource::KneeLeft:
			case DamageSource::KneeRight:
				PreventDamage = true;
			break;
			case DamageSource::HandCrawlLeft:
			case DamageSource::HandCrawlRight:
				PreventDamage = true;
			break;
		}
		if (PreventDamage) {
			// goal of this function is to deal heavily decreased damage on normal walk footsteps to actors
			// so it won't look silly by dealing 30 damage by briefly colliding with others
			if (difference > 1.4) {
				InflictSizeDamage(giant, tiny, difference * 0.35);
			} 
			return false;
		}
		return true;
	}

	bool ApplyHighHeelBonus(Actor* giant, DamageSource cause) {
		bool HighHeel = false;
		switch (cause) {
			case DamageSource::CrushedRight:
				HighHeel = true;
			break;
			case DamageSource::CrushedLeft:
				HighHeel = true;
			break;
			case DamageSource::WalkRight:
				HighHeel = true;
			break;
			case DamageSource::WalkLeft:
				HighHeel = true;
			break;
		}
		return HighHeel;
	}

	void ModVulnerability(Actor* giant, Actor* tiny, float damage) {
		if (Runtime::HasPerkTeam(giant, "GrowingPressure")) {
			auto& sizemanager = SizeManager::GetSingleton();
			sizemanager.ModSizeVulnerability(tiny, damage * 0.0010);
		}
	}

	float HighHeels_PerkDamage(Actor* giant, DamageSource Cause) {
		float value = 1.0;
		bool perk = Runtime::HasPerkTeam(giant, "hhBonus");
		bool rumbling_feet = Runtime::HasPerkTeam(giant, "RumblingFeet");
		bool matches = false;

		switch (Cause) {
			case DamageSource::CrushedRight:
				matches = true;
			break;
			case DamageSource::CrushedLeft:
				matches = true;
			break;
			case DamageSource::WalkRight:
				matches = true;
			break;
			case DamageSource::WalkLeft:
				matches = true;
			break;
		}
		if (matches) {
			if (rumbling_feet) {
				value += 0.25; // 25% bonus damage if we have lvl 65 perk
			} if (perk) {
				value += 0.15; // 15% bonus damage if we have High Heels perk
			}
		}
		
		return value;
	}
}


namespace Gts {

	CollisionDamage& CollisionDamage::GetSingleton() noexcept {
		static CollisionDamage instance;
		return instance;
	}

	std::string CollisionDamage::DebugName() {
		return "CollisionDamage";
	}

	void CollisionDamage::DoFootCollision(Actor* actor, float damage, float radius, int random, float bbmult, float crush_threshold, DamageSource Cause, bool Right, bool ApplyCooldown, bool ignore_rotation) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto profiler = Profilers::Profile("CollisionDamageLeft: DoFootCollision_Left");
		auto& CollisionDamage = CollisionDamage::GetSingleton();
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor) * GetSizeFromBoundingBox(actor);
		float BASE_CHECK_DISTANCE = 180.0;
		float SCALE_RATIO = 1.15;
		float Calamity = 1.0;

		bool SMT = HasSMT(actor);
		if (SMT) {
			giantScale += 0.20;
			SCALE_RATIO = 0.7;
			Calamity = 3.0 * Get_Bone_Movement_Speed(actor, Cause); // larger range for shrinking radius with Tiny Calamity
		}

		float maxFootDistance = radius * giantScale;
		std::vector<NiPoint3> CoordsToCheck = GetFootCoordinates(actor, Right, ignore_rotation);

		if (!CoordsToCheck.empty()) {
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
				if (Cause != DamageSource::FootIdleL && Cause != DamageSource::FootIdleR) {
					for (auto footPoints: CoordsToCheck) {
						DebugAPI::DrawSphere(glm::vec3(footPoints.x, footPoints.y, footPoints.z), maxFootDistance, 300);
					}
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor) * GetSizeFromBoundingBox(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							bool DoDamage = true;

							auto model = otherActor->GetCurrent3D();
							
							if (model) {
								bool StopDamageLookup = false;
								for (auto point: CoordsToCheck) {
									if (!StopDamageLookup) {
										VisitNodes(model, [&nodeCollisions, &Calamity, &DoDamage, SMT, point, maxFootDistance, &StopDamageLookup](NiAVObject& a_obj) {
											float distance = (point - a_obj.world.translate).Length();
											if (distance - Collision_Distance_Override < maxFootDistance) {
												StopDamageLookup = true;
												nodeCollisions += 1;
												return false;
											} else if (SMT && distance - Collision_Distance_Override > maxFootDistance && distance < maxFootDistance*Calamity) {
												StopDamageLookup = true;
												nodeCollisions += 1;
												DoDamage = false;
												return false;
											}
											return true;
										});
									}
								}
							}
							if (nodeCollisions > 0) {
								//damage /= nodeCollisions;

								if (ApplyCooldown) { // Needed to fix Thigh Crush stuff
									auto& sizemanager = SizeManager::GetSingleton();
									bool OnCooldown = IsActionOnCooldown(otherActor, CooldownSource::Damage_Thigh);
									if (!OnCooldown) {
										Utils_PushCheck(actor, otherActor, Get_Bone_Movement_Speed(actor, Cause)); // pass original un-altered force
										CollisionDamage.DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, DoDamage);
										ApplyActionCooldown(otherActor, CooldownSource::Damage_Thigh);
									}
								} else {
									Utils_PushCheck(actor, otherActor, Get_Bone_Movement_Speed(actor, Cause)); // pass original un-altered force
									CollisionDamage.DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, DoDamage);
								}
							}
						}
					}
				}
			}
		}
	}

	void CollisionDamage::DoSizeDamage(Actor* giant, Actor* tiny, float damage, float bbmult, float crush_threshold, int random, DamageSource Cause, bool apply_damage) { // Applies damage and crushing
		auto profiler = Profilers::Profile("CollisionDamage: DoSizeDamage");
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (giant == tiny) {
			return;
		}
		if (!CanDoDamage(giant, tiny, true) || IsBetweenBreasts(tiny)) { // disallow 
			return;
		}

		auto& sizemanager = SizeManager::GetSingleton();

		float size_difference = GetSizeDifference(giant, tiny, SizeType::VisualScale, false, true);

		if (size_difference > 1.25) {
			if (Allow_Damage(giant, tiny, Cause, size_difference)) {
				float damagebonus = HighHeels_PerkDamage(giant, Cause); // 15% bonus HH damage if we have perk

				float vulnerability = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
				float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, SizeAttribute::Normal) * 0.30, 0.30, 999999.0);

				float highheelsdamage = 1.0;
				if (ApplyHighHeelBonus(giant, Cause)) {
					highheelsdamage = GetHighHeelsBonusDamage(giant, true);
				}

				float sprintdamage = 1.0; // default Sprint damage of 1.0
				float weightdamage = 1.0 + (giant->GetWeight()*0.01);

				if (giant->AsActorState()->IsSprinting()) {
					sprintdamage = 1.5 * sizemanager.GetSizeAttribute(giant, SizeAttribute::Sprint);
					damage *= 1.5;
				}

				float Might = 1.0 + Potion_GetMightBonus(giant);
				float damage_result = (damage * size_difference * damagebonus) * (normaldamage * sprintdamage) * (highheelsdamage * weightdamage) * vulnerability;

				damage_result *= Might;

				TinyCalamity_ShrinkActor(giant, tiny, damage_result * 0.66 * GetDamageSetting());

				if (giant->IsSneaking()) {
					damage_result *= 0.70;
				}

				SizeHitEffects::GetSingleton().BreakBones(giant, tiny, damage_result * bbmult, random);
				// ^ Chance to break bonues and inflict additional damage, as well as making target more vulerable to size damage

				if (!tiny->IsDead()) {
					float experience = std::clamp(damage_result/500, 0.0f, 0.05f);
					ModSizeExperience(giant, experience);
				}

				if (tiny->formID == 0x14 && GetAV(tiny, ActorValue::kStamina) > 2.0) {
					DamageAV(tiny, ActorValue::kStamina, damage_result * 2.0);
					damage_result -= GetAV(tiny, ActorValue::kStamina); // Reduce damage by stamina amount
					if (damage_result < 0) {
						damage_result = 0; // just to be safe and to not restore attributes
					}
					if (damage_result < GetAV(tiny, ActorValue::kStamina)) {
						return; // Fully protect against size-related damage
					}
				}
				if (apply_damage) {
					ModVulnerability(giant, tiny, damage_result);
					InflictSizeDamage(giant, tiny, damage_result);
					this->CrushCheck(giant, tiny, size_difference, crush_threshold, Cause);
				}
			}
		}
	}

	void CollisionDamage::CrushCheck(Actor* giant, Actor* tiny, float size_difference, float crush_threshold, DamageSource Cause) {
		bool CanBeCrushed = (
			GetAV(tiny, ActorValue::kHealth) <= 1.0 ||
			tiny->IsDead()
		);
		
		if (CanBeCrushed) {
			if (size_difference > Action_Crush * crush_threshold && CrushManager::CanCrush(giant, tiny)) {
				ModSizeExperience_Crush(giant, tiny, true);

				if (!tiny->IsDead()) {
					if (IsGiant(tiny)) {
						AdvanceQuestProgression(giant, tiny, QuestStage::Giant, 1, false);
					} else {
						AdvanceQuestProgression(giant, tiny, QuestStage::Crushing, 1, false);
					}
				} else {
					AdvanceQuestProgression(giant, tiny, QuestStage::Crushing, 0.25, false);
				}
				SetReanimatedState(tiny);

				CrushBonuses(giant, tiny);
				PrintDeathSource(giant, tiny, Cause);
				if (!LessGore()) {
					auto node = find_node(giant, GetDeathNodeName(Cause));
					if (!IsMechanical(tiny)) {
						PlayCrushSound(giant, node, true, StrongGore(Cause)); // Run Crush Sound task that will determine which exact type of crushing audio to play
					} 
				}

				SetBetweenBreasts(tiny, false);
				SetBeingHeld(tiny, false);

				CrushManager::GetSingleton().Crush(giant, tiny);
			}
		}
	}
}