#pragma once
// Handles the various methods of scaling an actor


using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	void set_target_scale(Actor& actor, float height);
	void set_target_scale(Actor* actor, float height);
	float get_target_scale(Actor& actor);
	float get_target_scale(Actor* actor);
	void mod_target_scale(Actor& actor, float amt);
	void mod_target_scale(Actor* actor, float amt);

	void set_max_scale(Actor& actor, float height);
	void set_max_scale(Actor* actor, float height);
	float get_max_scale(Actor& actor);
	float get_max_scale(Actor* actor);
	void mod_max_scale(Actor& actor, float amt);
	void mod_max_scale(Actor* actor, float amt);

	float get_visual_scale(Actor& actor);
	float get_visual_scale(Actor* actor);
	float get_natural_scale(Actor& actor);
	float get_natural_scale(Actor* actor);
	float get_neutral_scale(Actor* actor);
	float get_giantess_scale(Actor& actor);
	float get_giantess_scale(Actor* actor);
}
