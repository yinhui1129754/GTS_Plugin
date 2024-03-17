#include "events.hpp"
#include <format>
#include "data/time.hpp"
#include "Config.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	// Called on Live (non paused) gameplay
	void EventListener::Update() {
	}

	// Called on Papyrus OnUpdate
	void EventListener::PapyrusUpdate() {

	}

	// Called on Havok update (when processing hitjobs)
	void EventListener::HavokUpdate() {

	}

	// Called when the camera update event is fired (in the TESCameraState)
	void EventListener::CameraUpdate() {

	}

	// Called on game load started (not yet finished)
	// and when new game is selected
	void EventListener::Reset() {
	}

	// Called when game is enabled (while not paused)
	void EventListener::Enabled() {
	}

	// Called when game is disabled (while not paused)
	void EventListener::Disabled() {
	}

	// Called when a game is started after a load/newgame
	void EventListener::Start() {
	}

	// Called when all forms are loaded (during game load before mainmenu)
	void EventListener::DataReady() {

	}

	// Called when all forms are loaded (during game load before mainmenu)
	void EventListener::ResetActor(Actor* actor) {

	}

	// Called when an actor has an item equipped
	void EventListener::ActorEquip(Actor* actor) {

	}

	// Called when an actor has is fully loaded
	void EventListener::ActorLoaded(Actor* actor) {

	}

	// Called when a papyrus hit event is fired
	void EventListener::HitEvent(const TESHitEvent* evt) {
	}

	// Called when an actor is squashed underfoot
	void EventListener::UnderFootEvent(const UnderFoot& evt) {

	}

	// Fired when a foot lands
	void EventListener::OnImpact(const Impact& impact) {

	}

	// Fired when a highheel is (un)equiped or when an actor is loaded with HH
	void EventListener::OnHighheelEquip(const HighheelEquip& impact) {

	}

	// Fired when a perk is added
	void EventListener::OnAddPerk(const AddPerkEvent& evt) {

	}

	// Fired when a perk about to be removed
	void EventListener::OnRemovePerk(const RemovePerkEvent& evt) {

	}

	// Fired when a skyrim menu event occurs
	void EventListener::MenuChange(const MenuOpenCloseEvent* menu_event) {

	}

	// Fired when a actor animation event occurs
	void EventListener::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {

	}

	void EventDispatcher::AddListener(EventListener* listener) {
		if (listener) {
			EventDispatcher::GetSingleton().listeners.push_back(listener);
		}
	}

	void EventDispatcher::DoUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->Update();
		}
	}
	void EventDispatcher::DoPapyrusUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->PapyrusUpdate();
		}
	}
	void EventDispatcher::DoHavokUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->HavokUpdate();
		}
	}
	void EventDispatcher::DoCameraUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->CameraUpdate();
		}
	}
	void EventDispatcher::DoReset() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->Reset();
		}
	}
	void EventDispatcher::DoEnabled() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->Enabled();
		}
	}
	void EventDispatcher::DoDisabled() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->Disabled();
		}
	}
	void EventDispatcher::DoStart() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->Start();
		}
	}
	void EventDispatcher::DoDataReady() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->DataReady();
		}
	}
	void EventDispatcher::DoResetActor(Actor* actor) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->ResetActor(actor);
		}
	}
	void EventDispatcher::DoActorEquip(Actor* actor) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->ActorEquip(actor);
		}
	}
	void EventDispatcher::DoActorLoaded(Actor* actor) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->ActorLoaded(actor);
		}
	}
	void EventDispatcher::DoHitEvent(const TESHitEvent* evt) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->HitEvent(evt);
		}
	}
	void EventDispatcher::DoUnderFootEvent(const UnderFoot& evt) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->UnderFootEvent(evt);
		}
	}
	void EventDispatcher::DoOnImpact(const Impact& impact) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->OnImpact(impact);
		}
	}
	void EventDispatcher::DoHighheelEquip(const HighheelEquip& evt) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->OnHighheelEquip(evt);
		}
	}
	void EventDispatcher::DoAddPerk(const AddPerkEvent& evt)  {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->OnAddPerk(evt);
		}
	}
	void EventDispatcher::DoRemovePerk(const RemovePerkEvent& evt)  {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->OnRemovePerk(evt);
		}
	}
	void EventDispatcher::DoMenuChange(const MenuOpenCloseEvent* menu_event) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->MenuChange(menu_event);
		}
	}
	void EventDispatcher::DoActorAnimEvent(Actor* actor, const BSFixedString& a_tag, const BSFixedString& a_payload) {
		std::string tag = a_tag.c_str();
		std::string payload = a_payload.c_str();
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			auto profiler = Profilers::Profile(listener->DebugName());
			listener->ActorAnimEvent(actor, tag, payload);
		}
	}
	EventDispatcher& EventDispatcher::GetSingleton() {
		static EventDispatcher instance;
		return instance;
	}
}
