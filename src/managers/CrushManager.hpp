#pragma once
// Module that handles crushing others
#include "events.hpp"
#include "timer.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	enum class CrushState {
		Healthy,
		Crushing,
		Crushed
	};

	class CrushData {
		public:
			CrushData(Actor* giant);

			CrushState state;
			Timer delay;
			ActorHandle giant;
	};

	class CrushManager : public EventListener {
		public:
			[[nodiscard]] static CrushManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override; // PapyrusUpdate
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			static bool CanCrush(Actor* giant, Actor* tiny);
			static bool AlreadyCrushed(Actor* actor);
			static void Crush(Actor* giant, Actor* tiny);
		private:
			std::unordered_map<FormID, CrushData> data;
	};
}
