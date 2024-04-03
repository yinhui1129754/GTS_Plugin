#include "managers/animation/Utils/CooldownManager.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchObject.hpp"
#include "managers/damage/LaunchActor.hpp"
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
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
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

	const float LAUNCH_DAMAGE = 2.4f;
	const float LAUNCH_KNOCKBACK = 0.02f;
	const float BASE_CHECK_DISTANCE = 20.0f;

	

	float GetLaunchThreshold(Actor* giant) {
		float threshold = 8.0;
		if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
			threshold = 5.2;
		}
		return threshold;
	}
}


namespace Gts {

	LaunchActor& LaunchActor::GetSingleton() noexcept {
		static LaunchActor instance;
		return instance;
	}

	std::string LaunchActor::DebugName() {
		return "LaunchActor";
	}

	void LaunchActor::ApplyLaunchTo(Actor* giant, Actor* tiny, float force, float launch_power) {
		auto profiler = Profilers::Profile("Other: Launch Actors Decide");
		if (IsBeingHeld(giant, tiny)) {
			return;
		}
		if (IsBeingGrinded(tiny)) {
			return; // Disallow to launch if we're grinding an actor
		}

		float DamageMult = 0.6;
		float giantSize = get_visual_scale(giant);

		float startpower = 12.0 * (1.0 + Potion_GetMightBonus(giant)); // determines default power of launching someone

		float threshold = 6.0;
		float SMT = 1.0;

		bool OwnsPerk = false;

		if (HasSMT(giant)) {
			giantSize += 3.0;
			threshold = 0.8;
			force += 0.20;
		}
		float Adjustment = GetSizeFromBoundingBox(tiny);

		float sizeRatio = GetSizeDifference(giant, tiny, SizeType::VisualScale, false, true);

		float knockBack = LAUNCH_KNOCKBACK * giantSize * force;

		bool IsLaunching = IsActionOnCooldown(tiny, CooldownSource::Damage_Launch);
		if (!IsLaunching) {

		if (force >= 0.10) {
			float power = (1.0 * launch_power) / Adjustment;
			if (Runtime::HasPerkTeam(giant, "DisastrousTremor")) {
				DamageMult *= 2.0;
				OwnsPerk = true;
				power *= 1.5;
			}

			ApplyActionCooldown(tiny, CooldownSource::Damage_Launch);

			if (Runtime::HasPerkTeam(giant, "LaunchDamage") && CanDoDamage(giant, tiny, true)) {
				float damage = LAUNCH_DAMAGE * sizeRatio * force * DamageMult;
				InflictSizeDamage(giant, tiny, damage);
				if (OwnsPerk) { // Apply only when we have DisastrousTremor perk
					update_target_scale(tiny, -(damage / 1500) * GetDamageSetting(), SizeEffectType::kShrink);

					if (get_target_scale(tiny) < 0.12/Adjustment) {
						set_target_scale(tiny, 0.12/Adjustment);
					}
				}
			}

			PushActorAway(giant, tiny, 1.0);
			NiPoint3 Push = NiPoint3(0, 0, startpower * GetLaunchPower(giant, sizeRatio) * force * power);

			std::string name = std::format("LaunchOther_{}", tiny->formID);
			ActorHandle tinyHandle = tiny->CreateRefHandle();
			double startTime = Time::WorldTimeElapsed();

			TaskManager::Run(name, [=](auto& update){
				if (tinyHandle) {
					double endTime = Time::WorldTimeElapsed();
					auto tinyref = tinyHandle.get().get();
					if ((endTime - startTime) > 0.05) {
						ApplyManualHavokImpulse(tinyref, Push.x, Push.y, Push.z, 1.0);
						return false;
					}
					return true;
				}
				return true;
			});
		}
		}
	}

	void LaunchActor::ApplyLaunch_At(Actor* giant, float radius, float power, FootEvent kind) {
		if (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant)) {
			if (kind == FootEvent::Left) {
				LaunchActor::GetSingleton().LaunchLeft(giant, radius, power);
			}
			if (kind == FootEvent::Right) {
				LaunchActor::GetSingleton().LaunchRight(giant, radius, power);
			}
			if (kind == FootEvent::Butt) {
				auto ThighL = find_node(giant, "NPC L Thigh [LThg]");
				auto ThighR = find_node(giant, "NPC R Thigh [RThg]");
				if (ThighL && ThighR) {
					LaunchActor::LaunchAtNode(giant, radius, power, ThighL);
					LaunchActor::LaunchAtNode(giant, radius, power, ThighR);
				}
			} else if (kind == FootEvent::Breasts) {
				auto BreastL = find_node(giant, "NPC L Breast");
				auto BreastR = find_node(giant, "NPC R Breast");
				auto BreastL03 = find_node(giant, "L Breast03");
				auto BreastR03 = find_node(giant, "R Breast03");
				if (BreastL03 && BreastR03) {
					LaunchActor::LaunchAtNode(giant, radius, power, BreastL03);
					LaunchActor::LaunchAtNode(giant, radius, power, BreastR03);
				} else if (BreastL && BreastR) {
					LaunchActor::LaunchAtNode(giant, radius, power, BreastL);
					LaunchActor::LaunchAtNode(giant, radius, power, BreastR);
				}
			}
		}
	}


	void LaunchActor::LaunchAtNode(Actor* giant, float radius, float power, std::string_view node) {
		auto bone = find_node(giant, node);
		if (bone) {
			LaunchActor::FindLaunchActors(giant, radius, 0.0, power, bone);
		}
	}

	void LaunchActor::LaunchAtNode(Actor* giant, float radius, float power, NiAVObject* node) {
		LaunchActor::FindLaunchActors(giant, radius, 0.0, power, node);
	}

	void LaunchActor::FindLaunchActors(Actor* giant, float radius, float min_radius, float power, NiAVObject* node) {
		auto profiler = Profilers::Profile("Other: Launch Actor Crawl");
		if (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant)) {
			if (!node) {
				return;
			}
			if (!giant) {
				return;
			}
			float giantScale = get_visual_scale(giant);
			float launchdamage = 1.6;

			float SCALE_RATIO = GetLaunchThreshold(giant)/GetMovementModifier(giant);
			if (HasSMT(giant)) {
				SCALE_RATIO = 1.0/GetMovementModifier(giant);
				giantScale *= 1.5;
			}

			NiPoint3 NodePosition = node->world.translate;

			float maxDistance = BASE_CHECK_DISTANCE * radius * giantScale;
			// Make a list of points to check
			std::vector<NiPoint3> points = {
				NiPoint3(0.0, 0.0, 0.0), // The standard position
			};
			std::vector<NiPoint3> CrawlPoints = {};

			for (NiPoint3 point: points) {
				CrawlPoints.push_back(NodePosition);
			}

			for (auto point: CrawlPoints) {
				if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance, 600, {0.0, 0.0, 1.0, 1.0});
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			PushObjectsUpwards(giant, CrawlPoints, maxDistance, power);

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: CrawlPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxDistance) {
								if (min_radius > 0.0 && distance < min_radius) {
									return;
								}
								float force = 1.0 - distance / maxDistance;
								ApplyLaunchTo(giant, otherActor, force, power);
							}
						}
					}
				}
			}
		}
	}

	void LaunchActor::LaunchLeft(Actor* giant, float radius, float power) {
		auto profiler = Profilers::Profile("Other: Launch Actor Left");
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		float SCALE_RATIO = GetLaunchThreshold(giant)/GetMovementModifier(giant);
		if (HasSMT(giant)) {
			SCALE_RATIO = 1.0 / GetMovementModifier(giant);
			giantScale *= 1.5;
		}

		radius *= 1.0 + GetHighHeelsBonusDamage(giant) * 2.5;

		// Get world HH offset
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto leftFoot = find_node(giant, leftFootLookup);
		auto leftCalf = find_node(giant, leftCalfLookup);
		auto leftToe = find_node(giant, leftToeLookup);
		auto BodyBone = find_node(giant, bodyLookup);
		if (!leftFoot) {
			return;
		}
		if (!leftCalf) {
			return;
		}
		if (!leftToe) {
			return;
		}
		if (!BodyBone) {
			return; // CTD protection attempts
		}
		NiMatrix3 leftRotMat;
		{
			NiAVObject* foot = leftFoot;
			NiAVObject* calf = leftCalf;
			NiAVObject* toe = leftToe;
			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = forward.UnitCross(up);
			forward = up.UnitCross(right); // Reorthonalize

			leftRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = BASE_CHECK_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 600, {0.0, 0.0, 1.0, 1.0});
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			PushObjectsUpwards(giant, footPoints, maxFootDistance, power);

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
								ApplyLaunchTo(giant, otherActor, force, power);
							}
						}
					}
				}
			}
		}
	}



	void LaunchActor::LaunchRight(Actor* giant, float radius, float power) {
		auto profiler = Profilers::Profile("Other: Launch Actor Right");
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		float SCALE_RATIO = GetLaunchThreshold(giant)/GetMovementModifier(giant);
		if (HasSMT(giant)) {
			SCALE_RATIO = 1.0 / GetMovementModifier(giant);
			giantScale *= 1.5;
		}
		radius *= 1.0 + GetHighHeelsBonusDamage(giant) * 2.5;

		// Get world HH offset
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto rightFoot = find_node(giant, rightFootLookup);
		auto rightCalf = find_node(giant, rightCalfLookup);
		auto rightToe = find_node(giant, rightToeLookup);
		auto BodyBone = find_node(giant, bodyLookup);


		if (!rightFoot) {
			return;
		}
		if (!rightCalf) {
			return;
		}
		if (!rightToe) {
			return;
		}
		if (!BodyBone) {
			return; // CTD protection attempts
		}
		NiMatrix3 rightRotMat;
		{
			NiAVObject* foot = rightFoot;
			NiAVObject* calf = rightCalf;
			NiAVObject* toe = rightToe;

			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = up.UnitCross(forward);
			forward = right.UnitCross(up); // Reorthonalize

			rightRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = BASE_CHECK_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 600, {0.0, 0.0, 1.0, 1.0});
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			PushObjectsUpwards(giant, footPoints, maxFootDistance, power);

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
								ApplyLaunchTo(giant, otherActor, force, power);
							}
						}
					}
				}
			}
		}
	}
}
