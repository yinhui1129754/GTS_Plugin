#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace RE;

namespace Gts {
	/**
	 * The class which tracks gts size effects.
	 */
	class GtsManager : public EventListener  {
		public:

			/**
			 * Get the singleton instance of the <code>GtsManager</code>.
			 */
			[[nodiscard]] static GtsManager& GetSingleton() noexcept;

			float experiment = 1.0;

			virtual void OnAddPerk(const AddPerkEvent& evt) override;

			virtual std::string DebugName() override;
			virtual void Start() override;
			virtual void Update() override;

			// Reapply changes (used after reload events)
			void reapply(bool force = true);
			void reapply_actor(Actor* actor, bool force = true);
	};
}
