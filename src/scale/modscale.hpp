#pragma once
// Handles the various methods of scaling an actor


using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	enum SizeMethod {
		ModelScale = 0,
		RootScale = 1,
		Hybrid = 2,
		RefScale = 3,
	};

	// @ Sermit, do not call Get_Other_Scale, call get_natural_scale instead
	// get_natural_scale is much faster and safer as it uses the cache
	float Get_Other_Scale(Actor* actor);

	// Inital scale is loaded ONCE per game run
	// not once per save. We assume that any edits happen
	// as esp/nif edits
	float GetInitialScale(Actor* actor);
	// Should be called on save load and on swapping the scale mode
	void ResetToInitScale(Actor* actor);

	void set_ref_scale(Actor* actor, float target_scale);
	bool set_model_scale(Actor* actor, float target_scale);
	bool set_npcnode_scale(Actor* actor, float target_scale);

	float get_fp_scale(Actor* giant);
	float get_npcnode_scale(Actor* actor);
	float get_npcparentnode_scale(Actor* actor);
	float get_model_scale(Actor* actor);
	float get_ref_scale(Actor* actor);
	float get_scale(Actor* actor);
	bool set_scale(Actor* actor, float scale);
}
