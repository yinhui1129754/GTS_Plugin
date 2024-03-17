#pragma once
// Module that holds data that is not persistent across saves
#include "events.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct TempActorData {
		float base_height;
		float base_volume;
		float char_weight;
		float shoe_weight;
		float fall_start;
		float last_set_fall_start;
		float health_boost;
		float SMT_Bonus_Duration;
		float SMT_Penalty_Duration;
		float carryweight_boost;
		float FallTimer;
		float Hug_AnimSpeed;
		float Throw_Speed;
		float push_force;
		float otherScales;
		float WorldFov_Default;
		float FpFov_Default;
		float ButtCrushGrowthAmount;
		float ShrinkWeakness;
		float Rotation_X;
		

		bool Throw_WasThrown;
		bool is_teammate;
		bool can_do_vore;
		bool can_be_crushed;
		bool being_held;
		bool between_breasts;
		bool about_to_be_eaten;
		bool dragon_was_eaten;
		bool can_be_vored;
		bool being_foot_grinded;
		bool SMT_ReachedFullSpeed;
		bool OverrideCamera;
		bool WasReanimated;
		bool FPCrawling;
		bool FPProning;
		bool Overkilled;
		bool Protection;
		bool was_sneaking;
		

		bool DisableControls;

		float IsNotImmune;

		TESObjectREFR* disable_collision_with;
		TESObjectREFR* Throw_Offender;
	};

	class Transient : public EventListener {
		public:
			[[nodiscard]] static Transient& GetSingleton() noexcept;

			TempActorData* GetData(TESObjectREFR* object);
			TempActorData* GetActorData(Actor* actor);
			std::vector<FormID> GetForms();

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
		private:

			mutable std::mutex _lock;
			std::unordered_map<FormID, TempActorData> _actor_data;
	};
}
