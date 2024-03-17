#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct GrabData {
		public:
			GrabData(TESObjectREFR* tiny, float strength);
			void SetGrabbed(bool decide);
			bool GetGrabbed();
			TESObjectREFR* tiny;
			bool holding;
			float strength;
	};

	class Grab : public EventListener
	{
		public:
			[[nodiscard]] static Grab& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void RegisterEvents();
			static void RegisterTriggers();

			static void DetachActorTask(Actor* giant);
			static void AttachActorTask(Actor* giant, Actor* tiny);
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			// Streangth is meant to be for a calculation of
			// escape chance currently unused
			static void GrabActor(Actor* giant, TESObjectREFR* tiny, float strength);
			static void GrabActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);

			bool GetHolding(Actor* giant);
			static void SetHolding(Actor* giant, bool decide);
			// Get object being held
			static TESObjectREFR* GetHeldObj(Actor* giant);
			// Same as `GetHeldObj` but with a conversion to actor if possible
			static Actor* GetHeldActor(Actor* giant);
			std::unordered_map<Actor*, GrabData> data;
	};

	void StartRHandRumble(std::string_view tag, Actor& actor, float power, float halflife);
	void StartLHandRumble(std::string_view tag, Actor& actor, float power, float halflife);
	void StopRHandRumble(std::string_view tag, Actor& actor);
	void StopLHandRumble(std::string_view tag, Actor& actor);
}
