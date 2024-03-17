#pragma once
// Module that handles footsteps
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ContactListener : public hkpContactListener, public hkpWorldPostSimulationListener
	{

		public:
			void ContactPointCallback(const hkpContactPointEvent& a_event) override;

			void CollisionAddedCallback(const hkpCollisionEvent& a_event) override;

			void CollisionRemovedCallback(const hkpCollisionEvent& a_event) override;

			void PostSimulationCallback(hkpWorld* a_world) override;

			RE::NiPointer<RE::bhkWorld> world = nullptr;

			void detach();
			void attach(NiPointer<bhkWorld> world);
			void ensure_last();
			void sync_camera_collision_groups();
			void enable_biped_collision();
	};

	class ContactManager : public EventListener {
		public:
			[[nodiscard]] static ContactManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void HavokUpdate() override;
			void UpdateCameraContacts();

			ContactListener listener{};
	};
}
