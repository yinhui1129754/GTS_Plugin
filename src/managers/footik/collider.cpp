#include "managers/footik/collider.hpp"
#include "managers/GtsManager.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"

#include "colliders/RE.hpp"

#include "util.hpp"


using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace Gts {
	/*
	ColliderManager& ColliderManager::GetSingleton() noexcept {
		static ColliderManager instance;
		return instance;
	}

	std::string ColliderManager::DebugName() {
		return "ColliderManager";
	}

	void ColliderManager::Update() {
		auto playerCharacter = PlayerCharacter::GetSingleton();
		auto cell = playerCharacter->GetParentCell();
		if (!cell) {
			return;
		}

		if (cell != this->previous_cell) {
			this->FlagReset();
			this->previous_cell = cell;
		}

		std::uint64_t last_reset_frame = this->last_reset_frame.load();


		auto actors = find_actors();
		for (auto actor: actors) {
			if (actor->Is3DLoaded()) {
				ColliderActorData* actor_data = GetActorData(actor);
				if (actor_data) {
					actor_data->Update(actor, last_reset_frame);
				}
			}
		}

		auto itr = this->actor_data.begin();
		while (itr != this->actor_data.end())
		{
			bool found = (std::find(actors.begin(), actors.end(), itr->first) != actors.end());
			if (!found) {
				itr = this->actor_data.erase(itr);
			} else {
				itr++;
			}
		}
	}

	void ColliderManager::Reset() {
		std::unique_lock lock(this->_lock);
		this->actor_data.clear();
		this->last_reset_frame.store(0);
		this->previous_cell = nullptr;
	}

	void ColliderManager::ResetActor(Actor* actor) {
		ColliderActorData* result = nullptr;
		try {
			result = &this->actor_data.at(actor);
		} catch (const std::out_of_range& oor) {
			result = nullptr;
		}
		if (result) {
			result->FlagUpdate();
		}
	}

	void ColliderManager::ActorLoaded(Actor* actor) {
		this->ResetActor(actor);
	}

	void ColliderManager::HavokUpdate() {
		auto actors = find_actors();
		auto& manager = GtsManager::GetSingleton();
		for (auto actor: actors) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			ColliderActorData* actor_data = GetActorData(actor);
			if (actor_data) {
				float scale = get_visual_scale(actor); // /get_natural_scale(actor);
				if (fabs(scale - 1.0) <= 1e-4) {
					continue;
				}
				actor_data->ApplyPose(actor, scale);
			}
		}
	}

	ColliderActorData* ColliderManager::GetActorData(Actor* actor) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor;
		ColliderActorData* result = nullptr;
		try {
			result = &this->actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			this->actor_data.try_emplace(key, actor);
			try {
				result = &this->actor_data.at(key);
			} catch (const std::out_of_range& oor) {
				result = nullptr;
			}
		}
		return result;
	}

	void ColliderManager::FlagReset(){
		this->last_reset_frame.store(Time::FramesElapsed());
	}*/
}
