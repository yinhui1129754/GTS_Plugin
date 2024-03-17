#include "managers/cameras/tpState.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "UI/DebugAPI.hpp"


using namespace RE;
using namespace Gts;

namespace {
	float Modify_HalfLife() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();
		float result = sizemanager.GetCameraHalflife(player);
		return result;
	}
}

namespace Gts {
	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 pos = NiPoint3();
		auto player = GetCameraActor();
		if (player) {
			auto scale = get_visual_scale(player);
			auto boneTarget = this->GetBoneTarget();
			if (!boneTarget.boneNames.empty()) {
				auto player = GetCameraActor();
				if (player) {
					auto rootModel = player->Get3D(false);
					if (rootModel) {
						auto playerTrans = rootModel->world;
						playerTrans.scale = rootModel->parent ? rootModel->parent->world.scale : 1.0;  // Only do translation/rotation
						auto transform = playerTrans.Invert();
						NiPoint3 lookAt = CompuleLookAt(boneTarget.zoomScale);
						NiPoint3 localLookAt = transform*lookAt;
						this->smoothScale.halflife = Modify_HalfLife();
						this->smoothedBonePos.halflife = Modify_HalfLife();
						this->smoothScale.target = scale;
						pos += localLookAt * -1 * this->smoothScale.value;

						std::vector<NiAVObject*> bones = {};
						for (auto bone_name: boneTarget.boneNames) {
							auto node = find_node(player, bone_name);
							if (node) {
								bones.push_back(node);
							} else {
								log::error("Bone not found for camera target: {}", bone_name);
							}
						}

						NiPoint3 bonePos = NiPoint3();
						auto bone_count = bones.size();
						for (auto bone: bones) {
							auto worldPos = bone->world * NiPoint3();
							if (IsDebugEnabled()) {
								DebugAPI::DrawSphere(glm::vec3(worldPos.x, worldPos.y, worldPos.z), 1.0, 10, {1.0, 1.0, 0.0, 1.0});
							}
							auto localPos = transform * worldPos;
							bonePos += localPos * (1.0/bone_count);
						}
						NiPoint3 worldBonePos = playerTrans * bonePos;
						if (IsDebugEnabled()) {
							DebugAPI::DrawSphere(glm::vec3(worldBonePos.x, worldBonePos.y, worldBonePos.z), 1.0, 10, {0.0, 1.0, 0.0, 1.0});
						}
						smoothedBonePos.target = bonePos;
						pos += smoothedBonePos.value;
					}
				}
			}
		}
		return pos;
	}

	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPos) {
		NiPoint3 pos = this->GetPlayerLocalOffset(cameraPos);
		auto player = GetCameraActor();
		if (player) {
			auto scale = get_visual_scale(player);
			pos += this->ProneAdjustment(cameraPos)*scale;
		}
		return pos;
	}

	BoneTarget ThirdPersonCameraState::GetBoneTarget() {
		return BoneTarget();
	}

	NiPoint3 ThirdPersonCameraState::ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = Runtime::GetFloat("CalcProne");
		auto player = PlayerCharacter::GetSingleton();
		
		NiPoint3 result = NiPoint3();
		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}
