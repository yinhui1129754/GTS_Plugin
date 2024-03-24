#pragma once
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
// Module for accurate size-related damage

namespace Gts {
	class LaunchActor : public EventListener  {
		public:
			[[nodiscard]] static LaunchActor& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			void ApplyLaunch_At(Actor* giant, float radius, float power, FootEvent kind);

			void LaunchAtNode(Actor* giant, float radius, float power, std::string_view node);
			void LaunchAtNode(Actor* giant, float radius, float power, NiAVObject* node);

			void FindLaunchActors(Actor* giant, float radius, float min_radius, float power, NiAVObject* node);

			
			void LaunchLeft(Actor* giant, float radius, float power);
			void LaunchRight(Actor* giant, float radius, float power);
	};
}
