#include "managers/footik/collider_actor_data.hpp"
#include "utils/actorUtils.hpp"
#include "scale/scale.hpp"
#include "colliders/RE.hpp"
#include "data/world.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace {
	/*
	hkbFootIkDriver* GetFootIk(Actor* actor) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto footik_ref = character.footIkDriver.get();
					if (footik_ref) {
						auto footik = skyrim_cast<hkbFootIkDriver*>(footik_ref);
						if (footik) {
							return footik;
						}
					}
				}
			}
		}
		return nullptr;
	}*/
}

namespace Gts {
	/*ColliderActorData::ColliderActorData(Actor* actor) {
	}

	ColliderActorData::~ColliderActorData() {
	}

	void ColliderActorData::FlagUpdate() {
		this->last_update_frame.store(0);
	}

	void ColliderActorData::Reset() {
		this->last_update_frame.store(0);
	}

	void ColliderActorData::ApplyScale(const float& new_scale, const hkVector4& vec_scale) {
		this->footIkData.ApplyScale(new_scale, vec_scale);
	}

	void ColliderActorData::ApplyPose(Actor* actor, const float& new_scale) {
		auto model = actor->GetCurrent3D();
		if (model) {
			hkVector4 origin = hkVector4(model->world.translate * (World::WorldScale()));
			// this->ragdollData.ApplyPose(origin, new_scale);
		}
	}

	void ColliderActorData::Update(Actor* actor, std::uint64_t last_reset_frame) {
		auto charController = actor->GetCharController();
		auto footIk = GetFootIk(actor);

		bool needs_reset = this->last_update_frame.exchange(last_reset_frame) < last_reset_frame;
		bool footIkChanged = this->footIkData.ik != footIk;
		if (needs_reset || footIkChanged) {
			this->UpdateColliders(actor);
		}

		const float EPSILON = 1e-3;

		float scale_factor = get_visual_scale(actor);

		if ((fabs(this->last_scale - scale_factor) <= EPSILON) &&  !needs_reset) {
			return;
		}

		hkVector4 vecScale = hkVector4(scale_factor, scale_factor, scale_factor, scale_factor);

		// Prune any colliders that are not used anymore
		this->PruneColliders(actor);

		this->ApplyScale(scale_factor, vecScale);

		this->last_scale = scale_factor;
	}

	void ColliderActorData::UpdateColliders(Actor* actor) {

		// Search footIK
		auto ik = GetFootIk(actor);
		this->AddFootIk(ik);
	}

	void ColliderActorData::PruneColliders(Actor* actor) {
		this->footIkData.PruneColliders(actor);
	}

	void ColliderActorData::AddFootIk(hkbFootIkDriver* ik) {
		this->footIkData.UpdateColliders(ik);
	}*/
}