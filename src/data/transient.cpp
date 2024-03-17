#include "scale/modscale.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "spring.hpp"
#include "node.hpp"



using namespace SKSE;
using namespace RE;


namespace Gts {
	Transient& Transient::GetSingleton() noexcept {
		static Transient instance;
		return instance;
	}

	TempActorData* Transient::GetData(TESObjectREFR* object) {
		if (!object) {
			return nullptr;
		}
		auto key = object->formID;
		TempActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			return nullptr;
		}
		return result;
	}

	TempActorData* Transient::GetActorData(Actor* actor) {
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor->formID;
		try {
			auto no_discard = this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Try to add
			if (!actor) {
				return nullptr;
			}
			TempActorData result;
			auto bound = get_bound(actor);
			if (!bound) {
				return nullptr;
			}
			auto scale = get_scale(actor);
			if (scale < 0.0) {
				return nullptr;
			}
			float base_height_unit = bound->extents[2] * scale;
			float base_height_meters = unit_to_meter(base_height_unit);
			float fall_start = actor->GetPosition()[2];
			float last_set_fall_start = fall_start;
			float carryweight_boost = 0.0;
			float health_boost = 0.0;
			float SMT_Bonus_Duration = 0.0;
			float SMT_Penalty_Duration = 0.0;
			float FallTimer = 1.0;
			float Hug_AnimSpeed = 1.0;
			float Throw_Speed = 0.0;

			float push_force = 1.0;
			
			bool Throw_WasThrown = false;

			bool can_do_vore = true;
			bool can_be_crushed = true;
			bool dragon_was_eaten = false;
			bool can_be_vored = true;
			bool being_held = false;
			bool between_breasts = false;
			bool about_to_be_eaten = false;
			bool being_foot_grinded = false;
			bool SMT_ReachedFullSpeed = false;
			bool OverrideCamera = false;
			bool WasReanimated = false;
			bool FPCrawling = false;
			bool FPProning = false;
			bool Overkilled = false;
			bool Protection = false;
			bool was_sneaking = false;

			bool DisableControls = false;

			float IsNotImmune = 1.0;

			TESObjectREFR* disable_collision_with = nullptr;
			TESObjectREFR* Throw_Offender = nullptr;
			float otherScales = 1.0;
			float WorldFov_Default = 0;
			float FpFov_Default = 0;
			float ButtCrushGrowthAmount = 0;
			float ShrinkWeakness = 1.0;
			float Rotation_X = 0.0;

			// Volume scales cubically
			float base_volume = bound->extents[0] * bound->extents[1] * bound->extents[2] * scale * scale * scale;
			float base_volume_meters = unit_to_meter(base_volume);

			result.base_height = base_height_meters;
			result.base_volume = base_volume_meters;

			auto shoe = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
			float shoe_weight = 1.0;
			if (shoe) {
				shoe_weight = shoe->weight;
			}
			result.shoe_weight = shoe_weight;
			result.char_weight = actor->GetWeight();
			result.fall_start = fall_start;
			result.last_set_fall_start = last_set_fall_start;
			result.carryweight_boost = carryweight_boost;
			result.health_boost = health_boost;
			result.SMT_Bonus_Duration = SMT_Bonus_Duration;
			result.SMT_Penalty_Duration = SMT_Penalty_Duration;
			result.FallTimer = FallTimer;
			result.Hug_AnimSpeed = Hug_AnimSpeed;
			result.Throw_Speed = Throw_Speed;
			result.push_force = push_force;
			result.can_do_vore = can_do_vore;
			result.Throw_WasThrown = Throw_WasThrown;
			result.can_be_crushed = can_be_crushed;
			result.being_held = being_held;
			result.between_breasts = between_breasts;
			result.about_to_be_eaten = about_to_be_eaten;
			result.being_foot_grinded = being_foot_grinded;
			result.SMT_ReachedFullSpeed = SMT_ReachedFullSpeed;
			result.dragon_was_eaten = dragon_was_eaten;
			result.can_be_vored = can_be_vored;
			result.disable_collision_with = disable_collision_with;
			result.Throw_Offender = Throw_Offender;
			result.otherScales = otherScales;
			result.WorldFov_Default = WorldFov_Default;
			result.FpFov_Default = FpFov_Default;
			result.ButtCrushGrowthAmount = ButtCrushGrowthAmount;
			result.ShrinkWeakness = ShrinkWeakness;
			result.Rotation_X = Rotation_X;
			result.OverrideCamera = OverrideCamera;
			result.WasReanimated = WasReanimated;
			result.FPCrawling = FPCrawling;
			result.FPProning = FPProning;
			result.Overkilled = Overkilled;
			result.Protection = Protection;
			result.was_sneaking = was_sneaking;
			

			result.DisableControls = DisableControls;

			result.IsNotImmune = IsNotImmune;
			

			result.is_teammate = actor->formID != 0x14 && actor->IsPlayerTeammate();

			this->_actor_data.try_emplace(key, result);
		}
		return &this->_actor_data[key];
	}

	std::vector<FormID> Transient::GetForms() {
		std::vector<FormID> keys;
		keys.reserve(this->_actor_data.size());
		for(auto kv : this->_actor_data) {
			keys.push_back(kv.first);
		}
		return keys;
	}


	std::string Transient::DebugName() {
		return "Transient";
	}

	void Transient::Update() {
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}

			auto key = actor->formID;
			std::unique_lock lock(this->_lock);
			try {
				auto data = this->_actor_data.at(key);
				auto shoe = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
				float shoe_weight = 1.0;
				if (shoe) {
					shoe_weight = shoe->weight;
				}
				data.shoe_weight = shoe_weight;

				data.char_weight = actor->GetWeight();

				data.is_teammate = actor->formID != 0x14 && actor->IsPlayerTeammate();


			} catch (const std::out_of_range& oor) {
				continue;
			}
		}
	}
	void Transient::Reset() {
		log::info("Transient was reset");
		std::unique_lock lock(this->_lock);
		this->_actor_data.clear();
	}
	void Transient::ResetActor(Actor* actor) {
		std::unique_lock lock(this->_lock);
		if (actor) {
			auto key = actor->formID;
			this->_actor_data.erase(key);
		}
	}
}
