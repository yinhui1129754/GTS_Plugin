#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/TinyCalamity_Shrink.hpp"
#include "managers/animation/Sneak_Slam_Strong.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Grab_Sneak_Vore.hpp"
#include "managers/animation/Sneak_KneeCrush.hpp"
#include "managers/animation/Vore_Standing.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/Sneak_Swipes.hpp"
#include "managers/animation/StrongStomp.hpp"
#include "managers/animation/FootTrample.hpp"
#include "managers/animation/Grab_Attack.hpp"
#include "managers/animation/Grab_Throw.hpp"
#include "managers/animation/Sneak_Slam.hpp"
#include "managers/animation/Vore_Sneak.hpp"
#include "managers/animation/Vore_Crawl.hpp"
#include "managers/animation/ThighCrush.hpp"
#include "managers/animation/Grab_Vore.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/animation/BoobCrush.hpp"
#include "managers/animation/FootGrind.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/animation/Crawling.hpp"
#include "managers/animation/HugHeal.hpp"
#include "managers/animation/Proning.hpp"
#include "managers/animation/Compat.hpp"
#include "managers/animation/Growth.hpp"
#include "managers/animation/Shrink.hpp"
#include "managers/animation/Kicks.hpp"
#include "managers/animation/Stomp.hpp"
#include "managers/animation/Grab.hpp"
#include "utils/InputFunctions.hpp"
#include "data/persistent.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;
using namespace std;

namespace Gts {
	AnimationEventData::AnimationEventData(Actor& giant, TESObjectREFR* tiny) : giant(giant), tiny(tiny) {
	}
	AnimationEvent::AnimationEvent(std::function<void(AnimationEventData&)> a_callback,  std::string a_group) : callback(a_callback), group(a_group) {
	}
	TriggerData::TriggerData( std::vector< std::string_view> behavors,  std::string_view group) : behavors({}), group(group) {
		for (auto& sv: behavors) {
			this->behavors.push_back(std::string(sv));
		}
	}

	AnimationManager& AnimationManager::GetSingleton() noexcept {
		static AnimationManager instance;
		return instance;
	}

	std::string AnimationManager::DebugName() {
		return "AnimationManager";
	}

	void AnimationManager::DataReady() {
		AnimationStomp::RegisterEvents();
		AnimationStomp::RegisterTriggers();

		AnimationStrongStomp::RegisterEvents();
		AnimationStrongStomp::RegisterTriggers();

		AnimationThighSandwich::RegisterEvents();
		AnimationThighSandwich::RegisterTriggers();

		AnimationThighCrush::RegisterEvents();
		AnimationThighCrush::RegisterTriggers();

		AnimationCrawling::RegisterEvents();
		AnimationCrawling::RegisterTriggers();

		Animation_VoreStanding::RegisterEvents();
		Animation_VoreStanding::RegisterTriggers();

		Animation_VoreCrawl::RegisterEvents();
		Animation_VoreSneak::RegisterEvents();

		Animation_SneakSwipes::RegisterEvents();
		Animation_SneakSlam::RegisterEvents();
		Animation_SneakSlam_Strong::RegisterEvents();
		Animation_SneakSlam_FingerGrind::RegisterEvents();
		Animation_SneakSlam_FingerGrind::RegisterTriggers();

		Animation_TinyCalamity::RegisterEvents();
		Animation_TinyCalamity::RegisterTriggers();

		AnimationButtCrush::RegisterEvents();
		AnimationButtCrush::RegisterTriggers();

		AnimationSneakCrush::RegisterEvents();

		AnimationBoobCrush::RegisterEvents();


		Animation_GrabSneak_Vore::RegisterEvents();
		Animation_GrabVore::RegisterEvents();
		Animation_GrabThrow::RegisterEvents();
		Animation_GrabAttack::RegisterEvents();
		

		AnimationFootGrind::RegisterEvents();
		AnimationFootGrind::RegisterTriggers();

		AnimationFootTrample::RegisterEvents();
		AnimationFootTrample::RegisterTriggers();

		AnimationCompat::RegisterEvents();
		AnimationCompat::RegisterTriggers();

		AnimationProning::RegisterEvents();
		AnimationProning::RegisterTriggers();

		AnimationGrowth::RegisterEvents();
		AnimationGrowth::RegisterTriggers();

		AnimationShrink::RegisterEvents();
		AnimationShrink::RegisterTriggers();

		InputFunctions::RegisterEvents();

		AnimationKicks::RegisterEvents();

		HugShrink::RegisterEvents();
		HugShrink::RegisterTriggers();

		HugHeal::RegisterEvents();
		HugHeal::RegisterTriggers();

		Grab::RegisterEvents();
		Grab::RegisterTriggers();


	}

	void AnimationManager::Update() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			// Update fall behavor of player
			auto charCont = player->GetCharController();
			if (charCont) {
				player->SetGraphVariableFloat("GiantessVelocity", (charCont->outVelocity.quad.m128_f32[2] * 100)/get_giantess_scale(player));
			}
		}
	}

	void AnimationManager::Reset() {
		this->data.clear();
	}
	void AnimationManager::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	float AnimationManager::GetHighHeelSpeed(Actor* actor) {
		float Speed = 1.0;
		try {
			for (auto& [tag, data]: AnimationManager::GetSingleton().data.at(actor)) {
				Speed *= data.HHspeed;
			}
		} catch (std::out_of_range e) {
		}
		return Speed;
	}

	float AnimationManager::GetBonusAnimationSpeed(Actor* actor) {
		float totalSpeed = 1.0;
		try {
			for (auto& [tag, data]: AnimationManager::GetSingleton().data.at(actor)) {
				totalSpeed *= data.animSpeed;
			}
		} catch (std::out_of_range e) {
		}
		return totalSpeed;
	}

	void AnimationManager::AdjustAnimSpeed(float bonus) {
		auto player = PlayerCharacter::GetSingleton();
		try {
			for (auto& [tag, data]: AnimationManager::GetSingleton().data.at(player)) {
				if (data.canEditAnimSpeed) {
					data.animSpeed += (bonus*GetAnimationSlowdown(player));
				}
				data.animSpeed = std::clamp(data.animSpeed, 0.33f, 3.0f);
			}
		} catch (std::out_of_range e) {}
	}

	float AnimationManager::GetAnimSpeed(Actor* actor) {
		float speed = 1.0;
		if (!Persistent::GetSingleton().is_speed_adjusted) {
			return 1.0;
		}
		if (actor) {
			auto saved_data = Gts::Persistent::GetSingleton().GetData(actor);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					speed *= saved_data->anim_speed;
				}
			}

			try {
				float totalSpeed = 1.0;
				for (auto& [tag, data]: AnimationManager::GetSingleton().data.at(actor)) {
					totalSpeed *= data.animSpeed;
				}
				speed *= totalSpeed;
			} catch (std::out_of_range e) {
			}
		}
		return speed;
	}

	void AnimationManager::RegisterEvent( std::string_view name,  std::string_view group, std::function<void(AnimationEventData&)> func) {
		AnimationManager::GetSingleton().eventCallbacks.try_emplace(std::string(name), func, std::string(group));
		log::info("Registering Event: Name {}, Group {}", name, group);
	}

	void AnimationManager::RegisterTrigger( std::string_view trigger,  std::string_view group,  std::string_view behavior) {
		AnimationManager::RegisterTriggerWithStages(trigger, group, {behavior});
		log::info("Registering Trigger: {}, Group {}, Behavior {}", trigger, group, behavior);
	}

	void AnimationManager::RegisterTriggerWithStages( std::string_view trigger,  std::string_view group,  std::vector< std::string_view> behaviors) {
		if (behaviors.size() > 0) {
			AnimationManager::GetSingleton().triggers.try_emplace(std::string(trigger), behaviors, group);
			log::info("Registering Trigger With Stages: {}, Group {}", trigger, group);
		}
	}


	void AnimationManager::StartAnim( std::string_view trigger, Actor& giant) {
		AnimationManager::StartAnim(trigger, giant, nullptr);

	}
	void AnimationManager::StartAnim( std::string_view trigger, Actor* giant) {
		if (giant) {
			AnimationManager::StartAnim(trigger, *giant);
			//log::info("Starting Trigger {} for {}", trigger, giant->GetDisplayFullName());
		}
	}

	void AnimationManager::StartAnim(std::string_view trigger, Actor& giant, TESObjectREFR* tiny) {
		if (giant.formID == 0x14 && IsFirstPerson()) {
			return; // Don't start animations in FP, it's not supported.
		}
		try {
			auto& me = AnimationManager::GetSingleton();
			// Find the behavior for this trigger exit on catch if not
			/*bool Busy = IsGtsBusy(&giant);
			if (Busy) {
				log::info("GTS is currently busy");
				return; // < fixes an issue with animations that repeat self if we spam animations 
				// (press stomp, hold kick = perform stomps for example).
				// Current downside: this method breaks most anims that depend on that bool/set it to True (Hugs, Thigh Sandwich, Trampling, etc)
				// TO-DO: somehow fix it
			}*/

			auto& behavorToPlay = me.triggers.at(std::string(trigger));
			auto& group = behavorToPlay.group;
			// Try to create anim data for actor
			me.data.try_emplace(&giant);
			auto& actorData = me.data.at(&giant); // Must exists now
			// Create the anim data for this group if not present
			actorData.try_emplace(group, giant, tiny);
			// Run the anim
			//log::info("Playing Trigger {} for {}", trigger, giant.GetDisplayFullName());
			//log::info("Playing {}", behavorToPlay.behavors[0]);
			giant.NotifyAnimationGraph(behavorToPlay.behavors[0]);
		} catch (std::out_of_range) {
			log::error("Requested play of unknown animation named: {}", trigger);
			return;
		}
	}
	void AnimationManager::StartAnim(std::string_view trigger, Actor* giant, TESObjectREFR* tiny) {
		if (giant) {
			AnimationManager::StartAnim(trigger, *giant, tiny);
		}
	}

	void AnimationManager::NextAnim(std::string_view trigger, Actor& giant) {
		try {
			auto& me = AnimationManager::GetSingleton();
			// Find the behavior for this trigger exit on catch if not
			auto& behavorToPlay = me.triggers.at(std::string(trigger));
			auto& group = behavorToPlay.group;
			// Get the actor data OR exit on catch
			auto& actorData = me.data.at(&giant);
			// Get the event data of exit on catch
			auto& eventData = actorData.at(group);
			std::size_t currentTrigger = eventData.currentTrigger;
			// Run the anim
			if (behavorToPlay.behavors.size() < currentTrigger) {
				giant.NotifyAnimationGraph(behavorToPlay.behavors[currentTrigger]);
			}
		} catch (std::out_of_range) {
			return;
		}
	}
	void AnimationManager::NextAnim(std::string_view trigger, Actor* giant) {
		if (giant) {
			AnimationManager::NextAnim(trigger, *giant);
		}
	}

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		try {
			if (actor) {
				// Try to get the registerd anim for this tag
				auto& animToPlay = this->eventCallbacks.at(std::string(tag));
				// If data dosent exist then insert with default
				this->data.try_emplace(actor);
				auto& actorData = this->data.at(actor);
				auto group = animToPlay.group;
				// If data dosent exist this will insert it with default
				actorData.try_emplace(group, *actor, nullptr);
				// Get the data or the newly inserted data
				auto& data = actorData.at(group);
				// Call the anims function
				animToPlay.callback(data);
				// If the stage is 0 after an anim has been played then
				//   delete this data so that we can reset for the next anim
				if (data.stage == 0) {
					actorData.erase(group);
				}
			}
		} catch (std::out_of_range e) {}
	}

	// Get the current stage of an animation group
	std::size_t AnimationManager::GetStage(Actor& actor,  std::string_view group) {
		try {
			auto& me = AnimationManager::GetSingleton();
			return me.data.at(&actor).at(std::string(group)).stage;
		} catch (std::out_of_range e) {
			return 0;
		}
	}
	std::size_t AnimationManager::GetStage(Actor* actor,  std::string_view group) {
		if (actor) {
			return AnimationManager::GetStage(*actor, group);
		} else {
			return 0;
		}
	}

	// Check if any currently playing anim disabled the HHs
	bool AnimationManager::HHDisabled(Actor& actor) {
		try {
			auto& me = AnimationManager::GetSingleton();
			auto& actorData = me.data.at(&actor);
			for ( auto &[group, data]: actorData) {
				if (data.disableHH) {
					return true;
				}
			}
			return false;
		} catch (std::out_of_range e) {
			return false;
		}
	}
	bool AnimationManager::HHDisabled(Actor* actor) {
		if (actor) {
			return AnimationManager::HHDisabled(*actor);
		} else {
			return false;
		}
	}
}
