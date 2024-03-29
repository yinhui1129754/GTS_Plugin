#include "scale/height.hpp"
#include "scale/scale.hpp"
#include "managers/GtsManager.hpp"
#include "data/transient.hpp"

using namespace Gts;

namespace {
	float height_to_scale(Actor* actor, float height) {
		if (!actor) {
			return -1.0;
		}
		auto temp_actor_data = Transient::GetSingleton().GetData(actor);
		if (!temp_actor_data) {
			return -1.0;
		}
		return height / temp_actor_data->base_height;
	}

	float scale_to_height(Actor* actor, float scale) {
		if (!actor) {
			return -1.0;
		}
		auto temp_actor_data = Transient::GetSingleton().GetData(actor);
		if (!temp_actor_data) {
			return -1.0;
		}
		return scale * temp_actor_data->base_height;
	}

}

namespace Gts {
	void set_target_height(Actor* actor, float height) {
		float scale = height_to_scale(actor, height);
		set_target_scale(actor, scale);
	}

	float get_target_height(Actor* actor) {
		float scale = get_target_scale(actor);
		return scale_to_height(actor, scale);
	}

	void mod_target_height(Actor* actor, float amt) {
		float current_scale = get_target_scale(actor);
		float current_height = scale_to_height(actor, current_scale);
		float target_height = (current_height + amt);
		float target_scale = height_to_scale(actor, target_height);
		float scale_delta = target_scale - current_scale;
		mod_target_scale(actor, scale_delta);
	}

	void set_max_height(Actor* actor, float height) {
		float scale = height_to_scale(actor, height);
		set_max_scale(actor, scale);
	}

	float get_max_height(Actor* actor) {
		float scale = get_max_scale(actor);
		return scale_to_height(actor, scale);
	}
	void mod_max_height(Actor* actor, float amt) {
		float current_scale = get_max_scale(actor);
		float current_height = scale_to_height(actor, current_scale);
		float target_height = (current_height + amt);
		float target_scale = height_to_scale(actor, target_height);
		float scale_delta = target_scale - current_scale;
		mod_max_scale(actor, scale_delta);
	}

	float get_visual_height(Actor* actor) {
		float scale = get_visual_scale(actor);
		return scale_to_height(actor, scale);
	}

	float get_giantess_height(Actor* actor) {
		float scale = get_giantess_scale(actor);
		return scale_to_height(actor, scale);
	}

	float get_base_height(Actor* actor) {
		auto temp_actor_data = Transient::GetSingleton().GetData(actor);
		if (!temp_actor_data) {
			return -1.0;
		}
		return temp_actor_data->base_height;
	}

	float get_bounding_box_to_mult(Actor* actor) {
		auto nif_bb = get_bound(actor);
		if (nif_bb) {
			auto nif_dim = nif_bb->extents;
			float x = nif_dim.x;
			float y = nif_dim.y;
			float z = nif_dim.z;
			float natural_scale = std::clamp(get_natural_scale(actor), 0.1f, 1.0f);

			float box = pow(x*y*z/(22.0f*14.0f*64.0f), 1.0f/3.0f) * natural_scale;
			//log::info("Found bounds for {}, bounds :{}", actor->GetDisplayFullName(), Vector2Str(nif_dim));
			//log::info("Value: {}, NS: {}", box, natural_scale);
			return box;
		}
		return 1.0;
	}

	float get_bounding_box_z(Actor* actor) {
		auto nif_bb = get_bound(actor);
		if (nif_bb) {
			auto nif_dim = nif_bb->extents;
			return nif_dim.z;
		}
		return 0.0;
	}
}
