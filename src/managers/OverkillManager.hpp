#pragma once
// Module that handles overkilling others
#include "events.hpp"
#include "timer.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	enum class OverkillState {
		Healthy,
		Overkilling,
		Overkilled
	};

	class OverkillData {
		public:
			OverkillData(Actor* giant);

			OverkillState state;
			Timer delay;
			ActorHandle giant;
	};

	class OverkillManager : public EventListener {
		public:
			[[nodiscard]] static OverkillManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			static bool CanOverkill(Actor* giant, Actor* tiny);
			static bool AlreadyOverkilled(Actor* actor);
			static void Overkill(Actor* giant, Actor* tiny);
		private:
			std::unordered_map<FormID, OverkillData> data;
	};
}