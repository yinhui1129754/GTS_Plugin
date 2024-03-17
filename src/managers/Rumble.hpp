#pragma once
// Module that handles Rumble
#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	enum class RumpleState {
		RampingUp, // Just started
		Rumbling, // At max intensity keep it going
		RampingDown, // After stop is recieved we are now going to zero
		Still, // means we are done and should clean up
	};

	// Holds rumble data
	class RumbleData {
		public:
			RumbleData(float intensity, float duration, float halflife, std::string node);
			RumbleData(float intensity, float duration, float halflife, std::string_view node);
			void ChangeTargetIntensity(float intensity);
			void ChangeDuration(float duration);

			RumpleState state;
			float duration; // Value of 0 means keep going until stopped
			Spring currentIntensity;
			std::string node;
			double startTime;
	};

	// Holds all rumble data for an actor
	// This is needed because an actor can have many sources of rumble
	class ActorRumbleData {
		public:
			ActorRumbleData();
			Timer delay;
			// Tagged rumble data
			std::unordered_map<std::string, RumbleData> tags;
	};

	// Rumble for all actors
	class GRumble : public EventListener {
		public:
			[[nodiscard]] static GRumble& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			virtual void Update() override;

			// Use this to start a rumble.
			static void Start(std::string_view tag, Actor* giant, float intensity, float halflife, std::string_view node);
			// Use this to start a rumble. Without Node name will happen at NPC Root Node
			static void Start(std::string_view tag, Actor* giant, float intensity, float halflife);
			// Use this to stop a rumble. The tag must be the same as given in start
			static void Stop(std::string_view tag, Actor* giant);

			// Same as Start except with a duration (can still use Stop to end it early)
			static void For(std::string_view tag, Actor* giant, float intensity, float halflife, std::string_view node, float duration);

			// A quick rumble. This should be a short instance like a single stomp. May not be for one frame but will be short
			// - To Sermit: This is currently set to 1.0s but can tinker with it
			static void Once(std::string_view tag, Actor* giant, float intensity, float halflife, std::string_view node);

			// Without node name will happen at NPC Root Node
			static void Once(std::string_view tag, Actor* giant, float intensity, float halflife);
		private:
			std::unordered_map<Actor*, ActorRumbleData> data;
	};
}