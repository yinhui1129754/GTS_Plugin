#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Crawling.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"


namespace {
	void SpawnCrawlParticle(Actor* actor, float scale, NiPoint3 position) {
		SpawnParticle(actor, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), position, scale * 1.8, 7, nullptr);
	}

	std::string_view GetImpactNode(CrawlEvent kind) {
		if (kind == CrawlEvent::RightKnee) {
			return "NPC R Calf [RClf]";
		} else if (kind == CrawlEvent::LeftKnee) {
			return "NPC L Calf [LClf]";
		} else if (kind == CrawlEvent::RightHand) {
			return "NPC R Finger20 [RF20]";
		} else if (kind == CrawlEvent::LeftHand) {
			return "NPC L Finger20 [LF20]";
		} else {
			return "NPC L Finger20 [LF20]";
		}
	}
}

namespace Gts {

	void DoCrawlingSounds(Actor* actor, float scale, NiAVObject* node, FootEvent foot_kind) { // Call crawling sounds
		if (actor) {
			auto profiler = Profilers::Profile("CrawlSounds");
			auto player = PlayerCharacter::GetSingleton();
			if (actor->formID == 0x14 && HasSMT(actor)) {
				scale *= 1.85;
			}
			float sprint_factor = 1.0;
			bool LegacySounds = Persistent::GetSingleton().legacy_sounds; // Determine if we should play old pre 2.00 update sounds
			bool sprinting = false;
			if (scale > 1.2 && !actor->AsActorState()->IsSwimming()) {
				float start_l = 1.2;
				float start_xl = 11.99;
				float start_xlJumpLand= 1.99;
				float start_xxl = 20.0;
				scale *= 0.60;
				if (actor->formID == 0x14 && IsFirstPerson()) { // Footsteps are quieter when in first person
					scale *= 0.70;
				}

				if (actor->AsActorState()->IsWalking()) {
					scale *= 0.75; // Walking makes you sound quieter
				}

				if (Runtime::GetBool("EnableGiantSounds")) {
					FootStepManager::PlayLegacySounds(node, foot_kind, scale, start_l, start_xl, start_xxl);
					return; // New Sounds are disabled for now
					if (!LegacySounds) {       // Play normal sounds
						FootStepManager::PlayNormalSounds(node, foot_kind, scale, sprint_factor, sprinting);
						return;
					} else if (LegacySounds) { // Play old sounds
						FootStepManager::PlayLegacySounds(node, foot_kind, scale, start_l, start_xl, start_xxl);
						return;
					}
				}
			}
		}
	}
	void DoCrawlingFunctions(Actor* actor, float scale, float multiplier, float damage, CrawlEvent kind, std::string_view tag, float launch_dist, float damage_dist, float crushmult, DamageSource Cause) { // Call everything
		std::string_view name = GetImpactNode(kind);

		auto node = find_node(actor, name);
		if (!node) {
			return; // Make sure to return if node doesn't exist, no CTD in that case
		}

		float SMT = 1.0;
		float minimal_scale = 1.5;

		LaunchActor::GetSingleton().FindLaunchActors(actor, launch_dist, damage_dist, multiplier, node); // Launch actors
		// Order matters here since we don't want to make it even stronger during SMT, so that's why SMT check is after this function
		
		if (actor->formID == 0x14) {
			if (HasSMT(actor)) {
				SMT = 2.5; // Stronger Camera Shake
				multiplier *= 1.8;
				minimal_scale = 1.0;
				scale += 0.75;
				damage *= 2.0;
			}
		}

		std::string rumbleName = std::format("{}{}", tag, actor->formID);
		GRumble::Once(rumbleName, actor, 0.90 * multiplier * SMT, 0.02, name); // Do Rumble

		DoDamageAtPoint(actor, damage_dist, damage, node, 20, 0.05, crushmult, Cause); // Do size-related damage
		DoCrawlingSounds(actor, scale, node, FootEvent::Left);                      // Do impact sounds

		if (scale >= minimal_scale && !actor->AsActorState()->IsSwimming()) {
			NiPoint3 node_location = node->world.translate;

			NiPoint3 ray_start = node_location + NiPoint3(0.0, 0.0, meter_to_unit(-0.05*scale)); // Shift up a little
			NiPoint3 ray_direction(0.0, 0.0, -1.0);
			bool success = false;
			float ray_length = meter_to_unit(std::max(1.05*scale, 1.05));
			NiPoint3 explosion_pos = CastRay(actor, ray_start, ray_direction, ray_length, success);

			if (!success) {
				explosion_pos = node_location;
				explosion_pos.z = actor->GetPosition().z;
			}
			if (actor->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects")) {
				SpawnCrawlParticle(actor, scale * multiplier, explosion_pos);
			}
			if (actor->formID != 0x14 && Runtime::GetBool("NPCSizeEffects")) {
				SpawnCrawlParticle(actor, scale * multiplier, explosion_pos);
			}
		}
	}

	void DoDamageAtPoint(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, DamageSource Cause) { // Apply damage to specific bone
		auto profiler = Profilers::Profile("Other: CrawlDamage");
		if (!node) {
			return;
		}
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = 1.25;
		if (HasSMT(giant)) {
			SCALE_RATIO = 0.9;
			giantScale *= 1.3;
		}

		NiPoint3 NodePosition = node->world.translate;

		float maxDistance = radius * giantScale;
		float CheckDistance = 220 * giantScale;
		// Make a list of points to check

		float damage_zones_applied = 0.0;

		std::vector<NiPoint3> points = {
			NiPoint3(0.0, 0.0, 0.0), // The standard position
		};
		std::vector<NiPoint3> CrawlPoints = {};

		for (NiPoint3 point: points) {
			CrawlPoints.push_back(NodePosition);
		}
		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
			for (auto point: CrawlPoints) {
				DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance);
			}
		}

		NiPoint3 giantLocation = giant->GetPosition();

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					for (auto point: CrawlPoints) {
						if ((actorLocation-giantLocation).Length() <= CheckDistance) {
							
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								VisitNodes(model, [&nodeCollisions, &force, NodePosition, maxDistance](NiAVObject& a_obj) {
									float distance = (NodePosition - a_obj.world.translate).Length();
									if (distance < maxDistance) {
										nodeCollisions += 1;
										force = 1.0 - distance / maxDistance;
										return false;
									}
									return true;
								});
							}
							if (nodeCollisions > 0) {
								damage_zones_applied += 1.0;
								if (damage_zones_applied < 1.0) {
									damage_zones_applied = 1.0; // just to be safe
								}
								damage /= damage_zones_applied;
								float aveForce = std::clamp(force, 0.14f, 0.70f);
								
								Utils_PushCheck(giant, otherActor, aveForce); 
								
								CollisionDamage::GetSingleton().DoSizeDamage(giant, otherActor, damage, bbmult, crushmult, random, Cause, true);
							}
						}
					}
				}
			}
		}
	}

	void ApplyAllCrawlingDamage(Actor* giant, int random, float bonedamage) { // Applies damage to all 4 crawl bones at once
		auto LC = find_node(giant, "NPC L Calf [LClf]");
		auto RC = find_node(giant, "NPC R Calf [RClf]");
		auto LH = find_node(giant, "NPC L Finger20 [LF20]");
		auto RH = find_node(giant, "NPC R Finger20 [RF20]");
		if (!LC) {
			return;
		}
		if (!RC) {
			return;
		}
		if (!LH) {
			return;
		}
		if (!RH) {
			return;
		} // CTD protection


		DoDamageAtPoint(giant, Radius_Crawl_KneeIdle, Damage_Crawl_Idle, LC, random, bonedamage, 2.5, DamageSource::KneeLeft);         // Call Left Calf
		DoDamageAtPoint(giant, Radius_Crawl_KneeIdle, Damage_Crawl_Idle, RC, random, bonedamage, 2.5, DamageSource::KneeRight);        // Call Right Calf

		if (!IsTransferingTiny(giant)) { // Only do if we don't have someone in our left hand
			DoDamageAtPoint(giant, Radius_Crawl_HandIdle, Damage_Crawl_Idle, LH, random, bonedamage, 2.5, DamageSource::HandCrawlLeft); // Call Left Hand
		}

		DoDamageAtPoint(giant, Radius_Crawl_HandIdle, Damage_Crawl_Idle, RH, random, bonedamage, 2.5, DamageSource::HandCrawlRight);    // Call Right Hand
	}
}