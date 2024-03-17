#pragma once
// Handles the various methods of scaling an actor


using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	void set_target_height(Actor* actor, float height);
	float get_target_height(Actor* actor);
	void mod_target_height(Actor* actor, float amt);

	void set_max_height(Actor* actor, float height);
	float get_max_height(Actor* actor);
	void mod_max_height(Actor* actor, float amt);

	float get_visual_height(Actor* actor);
	float get_giantess_height(Actor* actor);
	float get_base_height(Actor* actor);

	float get_bounding_box_to_mult(Actor* actor);
	float get_bounding_box_z(Actor* actor);
}
