#include "managers/MaxSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "colliders/RE.hpp"
#include "rays/raycast.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "utils/debug.hpp"
#include "utils/av.hpp"
#include "profiler.hpp"
#include "events.hpp"
#include "spring.hpp"
#include "timer.hpp"

namespace {
    float get_endless_height(Actor* actor) {
		float endless = 0.0;
		if (Runtime::HasPerk(actor, "ColossalGrowth")) {
			endless = 99999999.0;
		}
		return endless;
	}

    float get_default_size_limit(float NaturalScale, float BaseLimit) { // default sie limit for everyone
        float size_calc = NaturalScale + ((BaseLimit - 1.0f) * NaturalScale);
        float GetLimit = std::clamp(size_calc, NaturalScale, 99999999.0f);

        return GetLimit;
    }

    float get_mass_based_limit(Actor* actor, float NaturalScale) { // get mass based size limit for Player if using Mass Based mode
        float low_limit = get_endless_height(actor);
        if (low_limit < 2) {
            low_limit = Runtime::GetFloat("sizeLimit");
        }
        float size_calc = NaturalScale + (Runtime::GetFloat("GtsMassBasedSize") * NaturalScale);
        float GetLimit = std::clamp(size_calc, NaturalScale, low_limit);

        return GetLimit;
    }

    float get_follower_size_limit(float NaturalScale, float FollowerLimit) { // Self explanatory
        float size_calc = NaturalScale + ((FollowerLimit) * NaturalScale);
        float GetLimit = std::clamp(size_calc, NaturalScale * FollowerLimit, 99999999.0f);

        return GetLimit;
    }

    float get_npc_size_limit(float NaturalScale, float NPCLimit) { // get non-follower size limit
        float size_calc = NaturalScale + ((NPCLimit - 1.0f) * NaturalScale);
		float GetLimit = std::clamp(size_calc, NaturalScale * NPCLimit, 99999999.0f);

        return GetLimit;
    }
}

namespace Gts {
    void UpdateMaxScale() {
        auto profiler = Profilers::Profile("SizeManager: Update");
		for (auto actor: find_actors()) {
			// 2023 + 2024: TODO: move away from polling
			float Endless = 0.0;
			if (actor->formID == 0x14) {
				Endless = get_endless_height(actor);
			}

            float NaturalScale = get_natural_scale(actor);
            float QuestStage = Runtime::GetStage("MainQuest");
			float GameScale = game_get_scale_overrides(actor);

			

			float BaseLimit = Runtime::GetFloatOr("sizeLimit", 1.0);
            float NPCLimit = Runtime::GetFloatOr("NPCSizeLimit", 1.0); // 0 by default
			float IsMassBased = Runtime::GetIntOr("SelectedSizeFormula", 0.0); // Should DLL use mass based formula for Player?
			float FollowerLimit = Runtime::GetFloatOr("FollowersSizeLimit", 1.0); // 0 by default

            float GetLimit = get_default_size_limit(NaturalScale, BaseLimit); // Default size limit
			
			if (actor->formID == 0x14 && IsMassBased >= 1.0) { 
				GetLimit = get_mass_based_limit(actor, NaturalScale); // Apply Player Mass-Based max size
			} else if (QuestStage > 100 && FollowerLimit > 0.0 && FollowerLimit != 1.0 && actor->formID != 0x14 && IsTeammate(actor)) { 
				GetLimit = get_follower_size_limit(NaturalScale, FollowerLimit); // Apply Follower Max Size
			} else if (QuestStage > 100 && NPCLimit > 0.0 && NPCLimit != 1.0 && actor->formID != 0x14 && !IsTeammate(actor)) { 
                GetLimit = get_npc_size_limit(NaturalScale, NPCLimit); // Apply Other NPC's max size
			}

			float TotalLimit = (GetButtCrushSize(actor) + ((GetLimit * Potion_GetSizeMultiplier(actor)) * (1.0 + Ench_Aspect_GetPower(actor)))) / GameScale;
            //                ^ Add Butt Crush to base     ^          Multiply size with potions              ^ Aspect Of Giantess *'es it again

			if (get_max_scale(actor) < TotalLimit + Endless || get_max_scale(actor) > TotalLimit + Endless) {
				set_max_scale(actor, TotalLimit);
			}
		}
    }
}
