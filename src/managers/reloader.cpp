#include "managers/reloader.hpp"
#include "events.hpp"

using namespace SKSE;
using namespace RE;

namespace Gts {
	std::string ReloadManager::DebugName() {
		return "ReloadManager";
	}

	void ReloadManager::DataReady() {
		auto event_sources = ScriptEventSourceHolder::GetSingleton();
		if (event_sources) {
			event_sources->AddEventSink<TESHitEvent>(this);
			event_sources->AddEventSink<TESObjectLoadedEvent>(this);
			event_sources->AddEventSink<TESEquipEvent>(this);
			event_sources->AddEventSink<TESTrackedStatsEvent>(this);
			event_sources->AddEventSink<TESResetEvent>(this);
		}
		auto ui = UI::GetSingleton();
		if (ui) {
			ui->AddEventSink<MenuOpenCloseEvent>(this);
			log::info("Gts: successfully registered MenuOpenCloseEventHandler");
		} else {
			log::error("Gts: failed to register MenuOpenCloseEventHandler");
			return;
		}
	}

	ReloadManager& ReloadManager::GetSingleton() noexcept {
		static ReloadManager instance;
		return instance;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESHitEvent * evn, BSTEventSource<TESHitEvent>* dispatcher)
	{
		if (evn) {
			EventDispatcher::DoHitEvent(evn);
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESObjectLoadedEvent * evn, BSTEventSource<TESObjectLoadedEvent>* dispatcher)
	{
		if (evn) {
			auto* actor = TESForm::LookupByID<Actor>(evn->formID);
			if (actor) {
				EventDispatcher::DoActorLoaded(actor);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESResetEvent* evn, BSTEventSource<TESResetEvent>* dispatcher)
	{
		if (evn) {
			auto* object = evn->object.get();
			if (object) {
				auto* actor = TESForm::LookupByID<Actor>(object->formID);
				if (actor) {
					EventDispatcher::DoResetActor(actor);
				}
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESEquipEvent* evn, BSTEventSource<TESEquipEvent>* dispatcher)
	{
		if (evn) {
			auto* actor = TESForm::LookupByID<Actor>(evn->actor->formID);
			if (actor) {
				EventDispatcher::DoActorEquip(actor);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESTrackedStatsEvent* evn, BSTEventSource<TESTrackedStatsEvent>* dispatcher)
	{
		if (evn) {
			std::string_view stat = evn->stat;
			string dragon_stat = "Dragon Souls Collected";
			if (stat == dragon_stat) {
				EventDispatcher::DoDragonSoulAbsorption();
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const MenuOpenCloseEvent* a_event, BSTEventSource<MenuOpenCloseEvent>* a_eventSource)
	{
		if (a_event) {
			EventDispatcher::DoMenuChange(a_event);
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
