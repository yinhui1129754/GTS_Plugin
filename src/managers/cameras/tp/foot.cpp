#include "managers/cameras/tp/foot.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/highheel.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;

namespace {
	const float OFFSET = 0.24f * 70.0f; // About 24cm up
}

namespace Gts {
	void Foot::EnterState() {
		auto player = GetCameraActor();
		if (player) {
			float playerScale = get_visual_scale(player);
			if (playerScale > 0.0) {
				this->smoothScale.value = playerScale;
				this->smoothScale.target = playerScale;
				this->smoothScale.velocity = 0.0;
			}
		}
	}

	NiPoint3 Foot::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		auto player = GetCameraActor();
		float playerScale = get_visual_scale(player);

		NiPoint3 lookAt = CompuleLookAt(0.45); // float is zoom scale

		NiPoint3 footPos = this->GetFootPos();

		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto playerTrans = rootModel->world;
				playerTrans.scale = rootModel->parent ? rootModel->parent->world.scale : 1.0;  // Only do translation/rotation
				auto transform = playerTrans.Invert();
				NiPoint3 localLookAt = transform*lookAt;
				this->smoothScale.target = playerScale;
				return localLookAt * -1 * this->smoothScale.value + footPos;
			}
		}
		return NiPoint3();
	}

	NiPoint3 Foot::GetPlayerLocalOffsetProne(const NiPoint3& cameraPos)  {
		return this->GetPlayerLocalOffset(cameraPos); // No prone adjustment in foot mode
	}

	NiPoint3 Foot::GetFootPos() {
		const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
		const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
		auto player = GetCameraActor();
		if (player) {
			float playerScale = get_visual_scale(player);
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto playerTrans = rootModel->world;
				playerTrans.scale = rootModel->parent ? rootModel->parent->world.scale : 1.0;  // Only do translation/rotation
				auto transform = playerTrans.Invert();
				auto leftFoot = find_node(player, leftFootLookup);
				auto rightFoot = find_node(player, rightFootLookup);
				if (leftFoot != nullptr && rightFoot != nullptr) {
					auto leftPosLocal = transform * (leftFoot->world * NiPoint3());
					auto rightPosLocal = transform * (rightFoot->world * NiPoint3());
					this->smoothFootPos.target = (leftPosLocal + rightPosLocal) / 2.0;
					if (!IsCrawling(player)) {
						NiPoint3 highheelOffset = HighHeelManager::GetHHOffset(player);
						this->smoothFootPos.target.z += OFFSET*playerScale;
						if (highheelOffset.Length() > 1e-4) {
							this->smoothFootPos.target -= highheelOffset;
						}
					}
				}
			}
		}
		return this->smoothFootPos.value;
	}


}
