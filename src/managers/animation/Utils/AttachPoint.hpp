#pragma once
#include "RE/N/NiMatrix3.h"
#include "RE/N/NiPoint3.h"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
namespace {
	const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "AnimObjectB";
	const std::string_view rightToeLookup = "AnimObjectB";
	const std::string_view bodyLookup = "NPC Spine1 [Spn1]";


	NiPoint3 CastRayDownwards(Actor* tiny) {
		bool success = false;
		NiPoint3 ray_start = tiny->GetPosition();
		ray_start.z += 40.0; // overrize .z with tiny .z + 40, so ray starts from above a bit
		NiPoint3 ray_direction(0.0, 0.0, -1.0);

		float ray_length = 180;

		NiPoint3 endpos = CastRayStatics(tiny, ray_start, ray_direction, ray_length, success);
		if (success) {
			return endpos;
		}
		return tiny->GetPosition();
	}
}

namespace Gts {
	template<typename T, typename U>
	bool AttachTo_NoForceRagdoll(T& anyGiant, U& anyTiny, NiPoint3 point) {
		Actor* giant =  GetActorPtr(anyGiant);
		Actor* tiny =  GetActorPtr(anyTiny);

		if (!giant) {
			return false;
		}
		if (!tiny) {
			return false;
		}
		auto charcont = tiny->GetCharController();
		if (charcont) {
			charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
		}
		tiny->SetPosition(point, true);
		return true;
	}
	template<typename T, typename U>
	bool AttachTo(T& anyGiant, U& anyTiny, NiPoint3 point) {
		Actor* giant =  GetActorPtr(anyGiant);
		Actor* tiny =  GetActorPtr(anyTiny);

		if (!giant) {
			return false;
		}
		if (!tiny) {
			return false;
		}

		tiny->SetPosition(point, true);

		ForceRagdoll(tiny, false);

		auto charcont = tiny->GetCharController();
		if (charcont) {
			charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
		}

		return true;
	}
	template<typename T, typename U>
	bool AttachTo(T& anyGiant, U& anyTiny, std::string_view bone_name) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		auto bone = find_node(giant, bone_name);
		if (!bone) {
			return false;
		}
		return AttachTo(anyGiant, anyTiny, bone->world.translate);
	}

	template<typename T, typename U>
	bool AttachToObjectA(T& anyGiant, U& anyTiny) {
		return AttachTo(anyGiant, anyTiny, "AnimObjectA");
	}

	template<typename T, typename U>
	bool AttachToObjectB(T& anyGiant, U& anyTiny) {
		return AttachTo(anyGiant, anyTiny, "AnimObjectB");
	}

	template<typename T, typename U>
	NiPoint3 AttachToObjectB_GetCoords(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return NiPoint3(0,0,0);
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return NiPoint3(0,0,0);
		}

		auto ObjectA = find_node(giant, "AnimObjectB");
		if (!ObjectA) {
			return NiPoint3(0,0,0);
		}

		NiPoint3 coords = ObjectA->world.translate;//foot->world*(rotMat*point);
		coords.z = CastRayDownwards(tiny).z; // Cast ray down to get precise ground position
		return coords;
		//return false;
	}

	template<typename T, typename U>
	NiPoint3 AttachToUnderFoot_Left(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return NiPoint3(0,0,0);
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return NiPoint3(0,0,0);
		}


		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto leftFoot = find_node(giant, leftFootLookup);
		auto leftCalf = find_node(giant, leftCalfLookup);
		auto leftToe = find_node(giant, leftToeLookup);
		if (!leftFoot) {
			return NiPoint3(0,0,0);
		}
		if (!leftCalf) {
			return NiPoint3(0,0,0);
		}
		if (!leftToe) {
			return NiPoint3(0,0,0);
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

		float hh = hhOffsetbase[2];
		// Make a list of points to check
		float Forward = 8 - (hh * 0.6); //Runtime::GetFloat("cameraAlternateX"); // 8 is ok, 5 with HH
		float UpDown = 9; //Runtime::GetFloat("cameraAlternateY"); // 8 too


		std::vector<NiPoint3> points = {
			NiPoint3(0, Forward - hh/45, -(UpDown + hh * 0.65)),
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
				NiPoint3 coords = leftFoot->world.translate;//foot->world*(rotMat*point);
				coords.z = CastRayDownwards(tiny).z; // Cast ray down to get precise ground position

				return coords;
				//return AttachTo(anyGiant, anyTiny, coords);
			}
		}
		return NiPoint3(0,0,0);
		//return false;
	}

	template<typename T, typename U>
	NiPoint3 AttachToUnderFoot_Right(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return NiPoint3(0,0,0);
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return NiPoint3(0,0,0);
		}

		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto rightFoot = find_node(giant, rightFootLookup);
		auto rightCalf = find_node(giant, rightCalfLookup);
		auto rightToe = find_node(giant, rightToeLookup);


		if (!rightFoot) {
			return NiPoint3(0,0,0);
		}
		if (!rightCalf) {
			return NiPoint3(0,0,0);
		}
		if (!rightToe) {
			return NiPoint3(0,0,0);
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

		float hh = hhOffsetbase[2];
		// Make a list of points to check
		float Forward = 8 - (hh * 0.6); //Runtime::GetFloat("cameraAlternateX"); // 8 is ok, 5 with HH
		float UpDown = 9; //Runtime::GetFloat("cameraAlternateY"); // 8 too


		std::vector<NiPoint3> points = {
			NiPoint3(0, Forward, -(UpDown + hh * 0.65)),
		};
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
				NiPoint3 coords = rightFoot->world.translate;//foot->world*(rotMat*point);
				coords.z = CastRayDownwards(tiny).z; // Cast ray down to get precise ground position
				return coords;
				//return AttachTo(anyGiant, anyTiny, coords);
			}
		}
		//return false
		return NiPoint3(0,0,0);
	}


	template<typename T, typename U>
	bool AttachToHand(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		auto FingerA = find_node(giant, "NPC L Finger02 [LF02]");
		if (!FingerA) {
			return false;
		}
		auto FingerB = find_node(giant, "NPC L Finger30 [LF30]");
		if (!FingerB) {
			return false;
		}
		NiPoint3 coords = (FingerA->world.translate + FingerB->world.translate) / 2.0;
		coords.z -= 3.0;
		return AttachTo(anyGiant, anyTiny, coords);
	}

	template<typename T, typename U>
	bool HugAttach(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}
		auto targetRootA = find_node(giant, "AnimObjectA");
		if (!targetRootA) {
			return false;
		}
		auto targetA = targetRootA->world.translate;

		float scaleFactor = get_visual_scale(tiny) / get_visual_scale(giant);

		NiPoint3 targetB = NiPoint3();
		std::vector<std::string_view> bone_names = {
			"NPC L Finger02 [LF02]",
			"NPC R Finger02 [RF02]",
			"L Breast02",
			"R Breast02"
		};
		std::uint32_t bone_count = bone_names.size();
		for (auto bone_name: bone_names) {
			auto bone = find_node(giant, bone_name);
			if (!bone) {
				Notify("Error: Breast Nodes could not be found.");
				Notify("Suggestion: install XP32 skeleton.");
				return false;
			}
			targetB += (bone->world * NiPoint3()) * (1.0/bone_count);
		}

		// scaleFactor = std::clamp(scaleFactor, 0.0f, 1.0f);
		auto targetPoint = targetA*(scaleFactor) + targetB*(1.0 - scaleFactor);
		if (IsDebugEnabled()) {
			DebugAPI::DrawSphere(glm::vec3(targetA.x, targetA.y, targetA.z), 2.0, 40, {1.0, 0.0, 0.0, 1.0});
			DebugAPI::DrawSphere(glm::vec3(targetB.x, targetB.y, targetB.z), 2.0, 40, {0.0, 1.0, 0.0, 1.0});
			DebugAPI::DrawSphere(glm::vec3(targetPoint.x, targetPoint.y, targetPoint.z), 2.0, 40, {0.0, 0.0, 1.0, 1.0});
		}

		return AttachTo(anyGiant, anyTiny, targetPoint);
	}

	template<typename T, typename U>
	bool AttachToCleavage(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}

		std::vector<std::string_view> bone_names = {
			"L Breast02",
			"R Breast02"
		};

		std::vector<std::string_view> center_bone_names = {
			"L Breast01",
			"R Breast01"
		};

		std::vector<std::string_view> up_bone_names = {
			"NPC L Clavicle [LClv]",
			"NPC R Clavicle [RClv]"
		};

		NiPoint3 clevagePos = NiPoint3();
		NiPoint3 centerBonePos = NiPoint3();
		NiPoint3 upBonePos = NiPoint3();

		for (auto bone_name: bone_names) {
			auto bone = find_node(giant, bone_name);
			if (!bone) {
				Notify("ERROR: Breast 02 bones not found");
				Notify("Install 3BB/XPMS32");
				return false;
			}
			if (IsDebugEnabled()) {
				DebugAPI::DrawSphere(glm::vec3(bone->world.translate.x, bone->world.translate.y, bone->world.translate.z), 2.0, 10, {1.0, 1.0, 1.0, 1.0});
			}
			clevagePos += (bone->world * NiPoint3()) * (1.0f / bone_names.size());
		}

		// Center bone
		for (auto bone_name: center_bone_names) {
			auto bone = find_node(giant, bone_name);
			if (!bone) {
				Notify("ERROR: Breast 01 bones not found");
				Notify("Install 3BB/XPMS32");
				return false;
			}
			if (IsDebugEnabled()) {
				DebugAPI::DrawSphere(glm::vec3(bone->world.translate.x, bone->world.translate.y, bone->world.translate.z), 2.0, 10, {1.0, 1.0, 1.0, 1.0});
			}
			centerBonePos += bone->world.translate  * (1.0f / center_bone_names.size());
		}

		// Up bone
		for (auto bone_name: up_bone_names) {
			auto bone = find_node(giant, bone_name);
			if (!bone) {
				Notify("ERROR: Clavicle bones not found");
				Notify("Install 3BB/XPMS32");
				return false;
			}
			if (IsDebugEnabled()) {
				DebugAPI::DrawSphere(glm::vec3(bone->world.translate.x, bone->world.translate.y, bone->world.translate.z), 2.0, 10, {1.0, 1.0, 1.0, 1.0});
			}
			upBonePos += bone->world.translate  * (1.0f / up_bone_names.size());
		}

		// Forward
		NiPoint3 forward = (clevagePos - centerBonePos);
		forward.Unitize();
		// Up
		NiPoint3 up = (upBonePos - centerBonePos);
		up.Unitize();
		// Sideways
		NiPoint3 sideways = up.Cross(forward);
		sideways.Unitize();
		// Reorthorg
		forward = up.Cross(sideways * -1.0);
		forward.Unitize();
		
		NiMatrix3 breastRotation = NiMatrix3(sideways, forward, up);


		// Manual offsets
		float offset_Y = Runtime::GetFloatOr("Cleavage_OffsetY", 1.0);
		float offset_Z = Runtime::GetFloatOr("Cleavage_OffsetZ", 1.0);

		// Sermite: Offset adjustment HERE
		NiPoint3 offset = NiPoint3(0.0, offset_Y, offset_Z);


		// Global space offset
		NiPoint3 globalOffset = breastRotation * offset;

		// rotate tiny to face the same direction as gts
		tiny->data.angle.z = giant->data.angle.z;

		clevagePos += globalOffset;

		if (IsDebugEnabled()) {
			DebugAPI::DrawSphere(glm::vec3(clevagePos.x, clevagePos.y, clevagePos.z), 2.0, 10, {1.0, 0.0, 0.0, 1.0});
		}

		return AttachTo(anyGiant, anyTiny, clevagePos);
	}
}
