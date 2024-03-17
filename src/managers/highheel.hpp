#pragma once
#include "data/transient.hpp"
#include "events.hpp"
#include "spring.hpp"
// Module that handles high heel

using namespace RE;

namespace Gts {
	struct HHData {
		Spring multiplier = Spring(1.0, 0.5); // Used to smotthly disable/enable the highheels
		bool wasWearingHh = false;
		NiPoint3 lastBaseHHOffset;
	};

	class HighHeelManager : public EventListener {
		public:
			[[nodiscard]] static HighHeelManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void HavokUpdate() override;
			virtual void ActorEquip(Actor* actor) override;
			virtual void ActorLoaded(Actor* actor) override;
			virtual void OnAddPerk(const AddPerkEvent& evt) override;

			static bool IsWearingHH(Actor* actor); // Checks if GetBaseHHOffset().Length() > 1e-4
			static void UpdateHHOffset(Actor* actor);
			static NiPoint3 GetBaseHHOffset(Actor* actor); // Unscaled HH as read from the shoe data
			static NiPoint3 GetHHOffset(Actor* actor); // Scaled HH

			void ApplyHH(Actor* actor, bool force);

			std::unordered_map<Actor*, HHData> data;
	};
}
