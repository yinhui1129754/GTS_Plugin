#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/TinyCalamity.hpp"
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
	const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "NPC L Toe0 [LToe]";
	const std::string_view rightToeLookup = "NPC R Toe0 [RToe]";
	const std::string_view bodyLookup = "NPC Spine1 [Spn1]";

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
			case DamageSource::KickedRight:
				HighHeel = true;
			break;
			case DamageSource::KickedLeft:
				HighHeel = true;
			break;
		}
		return HighHeel;
	}

	void ModVulnerability(Actor* giant, Actor* tiny, float damage) {
		if (!Runtime::HasPerkTeam(giant, "GrowingPressure")) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		sizemanager.ModSizeVulnerability(tiny, damage * 0.0015);
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
				value *= 1.25; // 25% bonus damage if we have lvl 60 perk
			} if (perk) {
				value *= 1.15; // 15% bonus damage if we have High Heels perk
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

	void CollisionDamage::DoFootCollision(Actor* actor, float damage, float radius, int random, float bbmult, float crush_threshold, DamageSource Cause, bool Right, bool ApplyCooldown) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto profiler = Profilers::Profile("CollisionDamageLeft: DoFootCollision_Left");
		auto& CollisionDamage = CollisionDamage::GetSingleton();
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor) * GetSizeFromBoundingBox(actor);
		float BASE_CHECK_DISTANCE = 120.0;
		float SCALE_RATIO = 1.15;
		float Calamity = 1.0;

		bool SMT = HasSMT(actor);
		if (SMT) {
			giantScale += 0.20;
			SCALE_RATIO = 0.7;
			Calamity = 3.0 * Get_Bone_Movement_Speed(actor, Cause); // larger range for shrinking radius with Tiny Calamity
		}

		// Get world HH offset
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		float offset_side = 0.0;//-0.8;

		std::string_view FootLookup = leftFootLookup;
		std::string_view CalfLookup = leftCalfLookup;
		std::string_view ToeLookup = leftToeLookup;

		if (Right) {
			FootLookup = rightFootLookup;
			CalfLookup = rightCalfLookup;
			ToeLookup = rightToeLookup;
			offset_side = 0.0;//0.8;
		}

		auto Foot = find_node(actor, FootLookup);
		auto Calf = find_node(actor, CalfLookup);
		auto Toe = find_node(actor, ToeLookup);
		if (!Foot) {
			return;
		}
		if (!Calf) {
			return;
		}
		if (!Toe) {
			return;
		}
		NiMatrix3 RotMat;
		{
			NiAVObject* foot = Foot;
			NiAVObject* calf = Calf;
			NiAVObject* toe = Toe;
			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 point_mid = (calf->world.translate + foot->world.translate) / 2; 		// Middle between knee and foot 			|---|---|
			NiPoint3 point_low = (point_mid + foot->world.translate) / 2; 					// Middle of middle between knee and foot 	|---|-|-|
			NiPoint3 point_low_2 = (point_low + foot->world.translate) / 2;  // Lowest point in general 					|---|--||
			NiPoint3 point_lowest = inverseFoot*((point_low_2 + foot->world.translate) / 2);

			NiPoint3 up = point_lowest;//inverseFoot*calf->world.translate;

			up = up / up.Length();

			NiPoint3 side = forward.UnitCross(up);
			forward = up.UnitCross(side); // Reorthonalize

			RotMat = NiMatrix3(side, forward, up);
		}

		float damage_zones_applied = 0.0;

		float maxFootDistance = radius * giantScale;
		float hh = hhOffsetbase[2] / get_npcparentnode_scale(actor);
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			// x = side, y = forward, z = up/down      
			NiPoint3(0.0, hh/10, -(0.25 + hh * 0.25)), 	// basic foot pos
			// ^ Point 1: ---()  
			NiPoint3(offset_side, 8.0 + hh/10, -hh * 1.10), // Toe point		
			// ^ Point 2: ()---   
			NiPoint3(0.0, hh/70, -hh * 1.10), // Underheel point 
			//            -----
			// ^ Point 3: ---()  
		};
		std::tuple<NiAVObject*, NiMatrix3> adjust(Foot, RotMat);

		for (const auto& [foot, rotMat]: {adjust}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance);
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
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &Calamity, &DoDamage, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											return false;
										} else if (distance > maxFootDistance && distance < maxFootDistance*Calamity) {
											nodeCollisions += 1;
											DoDamage = false;
											return false;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								damage_zones_applied += 1.0;
								if (damage_zones_applied < 1.0) {
									damage_zones_applied = 1.0; // just to be safe
								}
								damage /= damage_zones_applied;
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

		if (!Allow_Damage(giant, tiny, Cause, size_difference)) {
			return; 
		}

		float damagebonus = HighHeels_PerkDamage(giant, Cause); // 15% bonus HH damage if we have perk

		float vulnerability = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, SizeAttribute::Normal) * 0.30, 0.30, 999999.0);

		float highheelsdamage = 1.0;
		if (ApplyHighHeelBonus(giant, Cause)) {
			highheelsdamage = 1.0 + (GetHighHeelsBonusDamage(giant) * 5.0);
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
						AdvanceQuestProgression(giant, tiny, 7, 1, false);
					} else {
						AdvanceQuestProgression(giant, tiny, 3, 1, false);
					}
				}

				SetReanimatedState(tiny);

				CrushBonuses(giant, tiny);
				PrintDeathSource(giant, tiny, Cause);
				if (!LessGore()) {
					auto node = find_node(giant, GetDeathNodeName(Cause));
					if (node) {
						if (IsMechanical(tiny)) {
							return;
						} else {
							Runtime::PlaySoundAtNode("GtsCrushSound", giant, 1.0, 1.0, node);
						}
					} else {
						Runtime::PlaySound("GtsCrushSound", giant, 1.0, 1.0);
					}
				}

				CrushManager::GetSingleton().Crush(giant, tiny);
			}
		}
	}
}
