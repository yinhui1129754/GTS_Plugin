#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
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

		bool is_walking = (cause == DamageSource::WalkLeft || cause == DamageSource::WalkRight);
		bool knee_crawling = (cause == DamageSource::KneeLeft || cause == DamageSource::KneeRight);
		bool hand_crawling = (cause == DamageSource::HandCrawlLeft ||cause == DamageSource::HandCrawlRight);
		if (is_walking || knee_crawling || hand_crawling) {
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
		bool Crush = (cause == DamageSource::CrushedRight || cause == DamageSource::CrushedLeft);
		if (Crush) {
			return true;
		}
		bool Kick = (cause == DamageSource::KickedLeft || cause == DamageSource::KickedRight);
		if (Kick) {
			return true;
		}

		return false;
	}

	bool CanDoDamage(Actor* giant, Actor* tiny) {
		if (IsBeingHeld(tiny)) {
			return false;
		}
		bool NPC = Persistent::GetSingleton().NPCEffectImmunity;
		bool PC = Persistent::GetSingleton().PCEffectImmunity;
		if (NPC && giant->formID == 0x14 && (IsTeammate(tiny))) {
			return false; // Protect NPC's against player size-related effects
		}
		if (NPC && (IsTeammate(giant)) && (IsTeammate(tiny))) {
			return false; // Disallow NPC's to damage each-other if they're following Player
		}
		if (PC && (IsTeammate(giant)) && tiny->formID == 0x14) {
			return false; // Protect Player against friendly NPC's damage
		}
		return true;
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
		bool matches = (Cause == DamageSource::CrushedLeft || Cause == DamageSource::CrushedRight);
		bool walk = (Cause == DamageSource::WalkRight || Cause == DamageSource::WalkLeft);
		if (perk && (matches || walk)) {
			value += 0.15; // 15% bonus damage if we have High Heels perk
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

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 120.0;
		float SCALE_RATIO = 1.15;
		bool SMT = HasSMT(actor);
		if (SMT) {
			giantScale += 0.20;
			SCALE_RATIO = 0.7;
		}

		// Get world HH offset
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);


		float offset_side = -1.6;

		std::string_view FootLookup = leftFootLookup;
		std::string_view CalfLookup = leftCalfLookup;
		std::string_view ToeLookup = leftToeLookup;

		if (Right) {
			FootLookup = rightFootLookup;
			CalfLookup = rightCalfLookup;
			ToeLookup = rightToeLookup;
			offset_side = 1.6;
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

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = forward.UnitCross(up);
			forward = up.UnitCross(right); // Reorthonalize

			RotMat = NiMatrix3(right, forward, up);
		}

		float damage_zones_applied = 0.0;

		float maxFootDistance = radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(offset_side, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward and to the side
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
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
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if (SMT) {
							TinyCalamity_SeekActorForShrink_Foot(actor, otherActor, damage, radius, -1, 0.0, crush_threshold, Cause, Right, ApplyCooldown);
						}
						// This function is needed to Shrinking actors around foot. Mostly a duplicate of this function but without the damage
						// Can't do size damage debuff

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;
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
										Utils_PushCheck(actor, otherActor, force); // pass original un-altered force
										CollisionDamage.DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, true);
										ApplyActionCooldown(otherActor, CooldownSource::Damage_Thigh);
									}
								} else {
									Utils_PushCheck(actor, otherActor, force); // pass original un-altered force
									CollisionDamage.DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, true);
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
		if (!CanDoDamage(giant, tiny) || IsBetweenBreasts(giant)) { // disallow if 
			return;
		}

		auto& sizemanager = SizeManager::GetSingleton();

		float size_difference = GetSizeDifference(giant, tiny, SizeType::VisualScale, false, true);

		if (!Allow_Damage(giant, tiny, Cause, size_difference)) {
			return; 
		}

		float damagebonus = HighHeels_PerkDamage(giant, Cause); // 15% bonus HH damage if we have perk

		float vulnerability = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0) * 0.30, 0.30, 999999.0);

		float highheelsdamage = 1.0;
		if (ApplyHighHeelBonus(giant, Cause)) {
			highheelsdamage = 1.0 + (GetHighHeelsBonusDamage(giant) * 5);
		}

		float sprintdamage = 1.0; // default Sprint damage of 1.0
		float weightdamage = 1.0 + (giant->GetWeight()*0.01);

		TinyCalamity_CrushCheck(giant, tiny);

		if (giant->AsActorState()->IsSprinting()) {
			sprintdamage = 1.5 * sizemanager.GetSizeAttribute(giant, 1);
			damage *= 1.5;
		}

		float damage_result = (damage * size_difference * damagebonus) * (normaldamage * sprintdamage) * (highheelsdamage * weightdamage) * vulnerability;

		TinyCalamity_ShrinkActor(giant, tiny, damage_result);

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
