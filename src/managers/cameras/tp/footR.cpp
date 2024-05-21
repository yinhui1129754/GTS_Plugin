#include "managers/cameras/tp/footR.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/highheel.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;

namespace {
	const float OFFSET = -0.02f * 70.0f; // About 2cm down
}

namespace Gts {
	NiPoint3 FootR::GetFootPos() {
		const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
		auto player = GetCameraActor();
		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto playerTrans = rootModel->world;
				playerTrans.scale = rootModel->parent ? rootModel->parent->world.scale : 1.0;  // Only do translation/rotation
				auto transform = playerTrans.Invert();
				auto rightFoot = find_node(player, rightFootLookup);
				if (rightFoot != nullptr) {
					float playerScale = get_visual_scale(player);
					auto rightPosLocal = transform * (rightFoot->world * NiPoint3());
					this->smoothFootPos.target = rightPosLocal;
					NiPoint3 highheelOffset = HighHeelManager::GetHHOffset(player) * HighHeelManager::GetHHMultiplier(player);

					this->smoothFootPos.target.z += OFFSET*playerScale;
					if (highheelOffset.Length() > 1e-4) {
						this->smoothFootPos.target -= highheelOffset * 1.6;
					}
				}
			}
		}
		return this->smoothFootPos.value;
	}
}
