// Module that handles rumbling
#include "managers/animation/AnimationManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	RumbleData::RumbleData(float intensity, float duration, float halflife, float shake_duration, std::string node) :
		state(RumbleState::RampingUp),
		duration(duration),
		shake_duration(shake_duration),
		currentIntensity(Spring(0.0, halflife)),
		node(node),
		startTime(0.0) {
	}

	RumbleData::RumbleData(float intensity, float duration, float halflife, float shake_duration, std::string_view node) : RumbleData(intensity, duration, halflife, shake_duration, std::string(node)) {
	}

	void RumbleData::ChangeTargetIntensity(float intensity) {
		this->currentIntensity.target = intensity;
		this->state = RumbleState::RampingUp;
		this->startTime = 0.0;
	}
	void RumbleData::ChangeDuration(float duration) {
		this->duration = duration;
		this->state = RumbleState::RampingUp;
		this->startTime = 0.0;
	}

	ActorRumbleData::ActorRumbleData()  : delay(Timer(0.40)) {
	}

	Rumbling& Rumbling::GetSingleton() noexcept {
		static Rumbling instance;
		return instance;
	}

	std::string Rumbling::DebugName() {
		return "Rumbling";
	}

	void Rumbling::Reset() {
		this->data.clear();
	}
	void Rumbling::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void Rumbling::Start(std::string_view tag, Actor* giant, float intensity, float halflife, std::string_view node) {
		Rumbling::For(tag, giant, intensity, halflife, node, 0, 0.0);
	}
	void Rumbling::Start(std::string_view tag, Actor* giant, float intensity, float halflife) {
		Rumbling::For(tag, giant, intensity, halflife, "NPC COM [COM ]", 0, 0.0);
	}
	void Rumbling::Stop(std::string_view tagsv, Actor* giant) {
		string tag = std::string(tagsv);
		auto& me = Rumbling::GetSingleton();
		try {
			me.data.at(giant).tags.at(tag).state = RumbleState::RampingDown;
		} catch (std::out_of_range e) {}
	}

	void Rumbling::For(std::string_view tagsv, Actor* giant, float intensity, float halflife, std::string_view nodesv, float duration, float shake_duration) {
		std::string tag = std::string(tagsv);
		std::string node = std::string(nodesv);
		auto& me = Rumbling::GetSingleton();
		me.data.try_emplace(giant);
		me.data.at(giant).tags.try_emplace(tag, intensity, duration, halflife, shake_duration, node);
		// Reset if already there (but don't reset the intensity this will let us smooth into it)
		me.data.at(giant).tags.at(tag).ChangeTargetIntensity(intensity);
		me.data.at(giant).tags.at(tag).ChangeDuration(duration);
	}

	void Rumbling::Once(std::string_view tag, Actor* giant, float intensity, float halflife, std::string_view node, float shake_duration) {
		Rumbling::For(tag, giant, intensity, halflife, node, 1.0, shake_duration);
	}

	void Rumbling::Once(std::string_view tag, Actor* giant, float intensity, float halflife) {
		Rumbling::Once(tag, giant, intensity, halflife, "NPC Root [Root]", 0.0);
	}


	void Rumbling::Update() {
		auto profiler = Profilers::Profile("Rumble: Update");
		for (auto& [actor, data]: this->data) {
			// Update values based on time passed
			std::vector<std::string> tagsToErase = {};
			for (auto& [tag, rumbleData]: data.tags) {
				switch (rumbleData.state) {
					case RumbleState::RampingUp: {
						// Increasing intensity just let the spring do its thing
						if (fabs(rumbleData.currentIntensity.value - rumbleData.currentIntensity.target) < 1e-3) {
							// When spring is done move the state onwards
							rumbleData.state = RumbleState::Rumbling;
							rumbleData.startTime = Time::WorldTimeElapsed();
						}
						break;
					}
					case RumbleState::Rumbling: {
						// At max intensity
						rumbleData.currentIntensity.value = rumbleData.currentIntensity.target;
						if (Time::WorldTimeElapsed() > rumbleData.startTime + rumbleData.duration) {
							rumbleData.state = RumbleState::RampingDown;
						}
						break;
					}
					case RumbleState::RampingDown: {
						// Stoping the rumbling
						rumbleData.currentIntensity.target = 0; // Ensure ramping down is going to zero intensity
						if (fabs(rumbleData.currentIntensity.value) <= 1e-3) {
							// Stopped
							rumbleData.state = RumbleState::Still;
						}
						break;
					}
					case RumbleState::Still: {
						// All finished cleanup
						this->data.erase(actor);
						return;
					}
				}
			}

			// Now collect the data
			//    - Multiple effects can add rumble to the same node
			//    - We sum those effects up into cummulativeIntensity
			float duration_override = 0.0;
			std::unordered_map<NiAVObject*, float> cummulativeIntensity;
			for (const auto &[tag, rumbleData]: data.tags) {
				duration_override = rumbleData.shake_duration;
				auto node = find_node(actor, rumbleData.node);
				if (node) {
					cummulativeIntensity.try_emplace(node);
					cummulativeIntensity.at(node) += rumbleData.currentIntensity.value;
				}
			}
			// Now do the rumble
			//   - Also add up the volume for the rumble
			//   - Since we can only have one rumble (skyrim limitation)
			//     we do a weighted average to find the location to rumble from
			//     and sum the intensities
			NiPoint3 averagePos = NiPoint3(0.0, 0.0, 0.0);
			
			float totalWeight = 0.0;

			for (const auto &[node, intensity]: cummulativeIntensity) {
				auto& point = node->world.translate;
				averagePos = averagePos + point*intensity;
				totalWeight += intensity;

				if (get_visual_scale(actor) >= 6.0) {
					float volume = 4 * get_visual_scale(actor)/get_distance_to_camera(point);
					// Lastly play the sound at each node
					if (data.delay.ShouldRun()) {
						//log::info("Playing sound at: {}, Intensity: {}", actor->GetDisplayFullName(), intensity);
						Runtime::PlaySoundAtNode("RumbleWalkSound", actor, volume, 1.0, node);
					}
				}
			}

			averagePos = averagePos * (1.0 / totalWeight);
			ApplyShakeAtPoint(actor, 0.4 * totalWeight, averagePos, duration_override);

			// There is a way to patch camera not shaking more than once so we won't need totalWeight hacks, but it requires ASM hacks
			// Done by this mod: https://github.com/jarari/ImmersiveImpactSE/blob/b1e0be03f4308718e49072b28010c38c455c394f/HitStopManager.cpp#L67
			// Edit: seems to be unstable to do it
		}
	}
}
