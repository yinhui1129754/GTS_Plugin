#pragma once

#include "events.hpp"

#include <atomic>
#include "colliders/RE.hpp"
#include "managers/footik/collider_actor_data.hpp"
#include "events.hpp"


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	/*
	class ColliderManager : public EventListener {
		public:
			[[nodiscard]] static ColliderManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			virtual void Update() override;
			virtual void HavokUpdate() override;
			void Reset() override;
			void ResetActor(Actor* actor) override;
			void ActorLoaded(Actor* actor) override;

			void FlagReset();

			ColliderActorData* GetActorData(Actor* actor);
		private:
			mutable std::mutex _lock;
			std::unordered_map<Actor*, ColliderActorData > actor_data;
			std::atomic_uint64_t last_reset_frame = std::atomic_uint64_t(0);
			TESObjectCELL* previous_cell = nullptr;
	};*/
}