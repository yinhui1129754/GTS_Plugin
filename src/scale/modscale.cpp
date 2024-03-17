#include "scale/modscale.hpp"
#include "node.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace Gts;

namespace {
  struct InitialScales {
    float model;
    float npc;

    InitialScales() {
      throw std::exception("Cannot init a InitialScales without an actor");
    }

    InitialScales(Actor* actor) {
      model = get_model_scale(actor);
      npc = get_npcnode_scale(actor);
    }
  };


  // Global actor inital scales singleton
  std::unordered_map<RE::FormID, InitialScales>& GetInitialScales() {
    static std::unordered_map<RE::FormID, InitialScales> initScales;
    return initScales;
  }

  InitialScales& GetActorInitialScales(Actor* actor) {
    if (!actor) {
      throw std::exception("Actor must exist for GetInitialScale");
    }
    auto& initScales = GetInitialScales();
    auto id = actor->formID;
    initScales.try_emplace(id, actor);
    return initScales.at(id);
  }

  void UpdateInitScale(Actor* actor) {
    GetActorInitialScales(actor); // It's enough just to call this
  }

}

namespace Gts {
	// @ Sermit, do not call Get_Other_Scale, call get_natural_scale instead
	// get_natural_scale is much faster and safer as it uses the cache
	//
	// Get the current physical value for all nodes of the player
	// that we don't alter
	//
	// This one calls the NiNode stuff so should really be done
	// once per frame and cached
	//
	// This cache is stored in transient as `otherScales`
	float Get_Other_Scale(Actor* actor) {
		float ourScale = get_scale(actor);

		// Work with world scale to grab accumuated scales rather
		// than multiplying it ourselves
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name, false);
		float allScale = 1.0;
		if (node) {
			// Grab the world scale which includes all effects from root
			// to here (the lowest scalable node)
			allScale = node->world.scale;

			float worldScale = 1.0;
			auto rootnode = actor->Get3D(false);
			if (rootnode) {
				auto worldNode = rootnode->parent;

				if (worldNode) {
					worldScale = worldNode->world.scale;

					allScale /= worldScale; // Remove effects of a scaled world
					                        // never actually seen a seen a scaled world
					                        // but here it is just in case
				}
			}
		}
		return allScale / ourScale;
	}

  void ResetToInitScale(Actor* actor) {
    if (actor) {
      if (actor->Is3DLoaded()) {
        auto& initScale = GetActorInitialScales(actor);
        set_model_scale(actor, initScale.model);
        set_npcnode_scale(actor, initScale.npc);
      }
    }
  }

  float GetInitialScale(Actor* actor) {
    auto& initScale = GetActorInitialScales(actor);
    auto& size_method = Persistent::GetSingleton().size_method;
		switch (size_method) {
      // Only initial scales for these two methods since the racemenu
      // and refscale one can edit on the character edit menu and setscale
			case SizeMethod::ModelScale:
				return initScale.model;
			case SizeMethod::RootScale:
        return initScale.npc;
      default:
        return 1.0;
    }
  }

	void set_ref_scale(Actor* actor, float target_scale) {
		// This is how the game sets scale with the `SetScale` command
		// It is limited to x10 and messes up all sorts of things like actor damage
		// and anim speeds
		// Calling too fast also kills frames
		float refScale = static_cast<float>(actor->GetReferenceRuntimeData().refScale) / 100.0F;
		if (fabs(refScale - target_scale) > 1e-5) {
			actor->GetReferenceRuntimeData().refScale = static_cast<std::uint16_t>(target_scale * 100.0F);
			actor->DoReset3D(false);
		}
	}

	bool set_model_scale(Actor* actor, float target_scale) {
		// This will set the scale of the model root (not the root npc node)
		if (!actor->Is3DLoaded()) {
			return false;
		}
		bool result = false;

    	UpdateInitScale(actor); // This will update the inital scales BEFORE we alter them

		auto model = actor->Get3D(false);
		if (model) {
			result = true;
			model->local.scale = target_scale;
			update_node(model);
		}

		auto first_model = actor->Get3D(true);
		if (first_model) {
			result = true;
			first_model->local.scale = target_scale;
			update_node(first_model);
		}
		return result;
	}

	bool set_npcnode_scale(Actor* actor, float target_scale) {
		// This will set the scale of the root npc node
		string node_name = "NPC Root [Root]";
		bool result = false;

    	UpdateInitScale(actor); // This will update the inital scales BEFORE we alter them

		auto node = find_node(actor, node_name, false);
		if (node) {
			result = true;
			node->local.scale = target_scale;
			update_node(node);
		}

		auto first_node = find_node(actor, node_name, true);
		if (first_node) {
			result = true;
			first_node->local.scale = target_scale;
			update_node(first_node);
		}
		return result;
	}


	float get_fp_scale(Actor* giant) {
		auto data = Persistent::GetSingleton().GetData(giant);
		if (data) {
			return data->scaleOverride;
		}
		return -1.0;
	}

	float get_npcnode_scale(Actor* actor) {
		// This will get the scale of the root npc node
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name, false);
		if (node) {
			return node->local.scale;
		}
		auto first_node = find_node(actor, node_name, true);
		if (first_node) {
			return first_node->local.scale;
		}
		return -1.0;
	}

	float get_npcparentnode_scale(Actor* actor) {
		// This will get the scale of the root npc node
		// this is also called the race scale, since it is
		// the racemenu scale
		//
		// The name of it is variable. For actors it is NPC
		// but for others it is the creature name
		string node_name = "NPC Root [Root]";
		auto childNode = find_node(actor, node_name, false);
		if (!childNode) {
			childNode = find_node(actor, node_name, true);
			if (!childNode) {
				return -1.0;
			}
		}
		auto parent = childNode->parent;
		if (parent) {
			return parent->local.scale;
		}
		return -1.0; //
	}

	float get_model_scale(Actor* actor) {
		// This will set the scale of the root npc node
		if (!actor->Is3DLoaded()) {
			return -1.0;
		}

		auto model = actor->Get3D(false);
		if (model) {
			return model->local.scale;
		}
		auto first_model = actor->Get3D(true);
		if (first_model) {
			return first_model->local.scale;
		}
		return -1.0;
	}

	float get_ref_scale(Actor* actor) {
		// This function reports same values as GetScale() in the console, so it is a value from SetScale() command
		// Used inside: GtsManager.cpp - apply_height
		//              Scale.cpp   -  get_natural_scale   
		if (actor->formID == 0x14 || IsTeammate(actor)) {
			log::info("Ref scale of {} is {}", actor->GetDisplayFullName(), static_cast<float>(actor->GetReferenceRuntimeData().refScale) / 100.0F);
		}
		return static_cast<float>(actor->GetReferenceRuntimeData().refScale) / 100.0F;
		
	}

	float get_scale(Actor* actor) {
		auto& size_method = Persistent::GetSingleton().size_method;
		switch (size_method) {
			case SizeMethod::ModelScale:
				return get_model_scale(actor);
				break;
			case SizeMethod::RootScale:
				return get_npcnode_scale(actor);
				break;
			case SizeMethod::RefScale:
				return get_ref_scale(actor);
				break;
			case SizeMethod::Hybrid:
				if (actor->formID == 0x14) {
					return get_npcnode_scale(actor);
				} else {
					return get_model_scale(actor);
				}
			default:
				return -1.0;
		}
	}

	bool set_scale(Actor* actor, float scale) {
		auto& size_method = Persistent::GetSingleton().size_method;
		switch (size_method) {
			case SizeMethod::ModelScale:
				return set_model_scale(actor, scale);
				break;
			case SizeMethod::RootScale:
				return set_npcnode_scale(actor, scale);
				break;
			case SizeMethod::RefScale:
				set_ref_scale(actor, scale);
				return true;
				break;
			case SizeMethod::Hybrid:
				if (actor->formID == 0x14) {
					return set_npcnode_scale(actor, scale);
				} else {
					return set_model_scale(actor, scale);
				}
				break;
		}
		return false;
	}
}
