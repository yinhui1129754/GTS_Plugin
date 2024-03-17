#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/explosion.hpp"
#include "managers/modevent.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "events.hpp"


#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	FootEvent get_foot_kind(Actor* actor, std::string_view tag) {
		auto profiler = Profilers::Profile("Impact: Get Foot Kind");
		FootEvent foot_kind = FootEvent::Unknown;
		bool is_jumping = actor ? IsJumping(actor) : false;
		bool in_air = actor ? actor->IsInMidair() : false;
		bool hugging = actor ? IsHuggingFriendly(actor) : false; 
		// Hugging is needed to fix missing footsteps once we do friendly release
		// Footsteps aren't seen by the dll without it (because actor is in air)

		bool allow = ((!is_jumping && !in_air) || hugging);

		if (matches(tag, ".*Foot.*Left.*") && allow) {
			foot_kind = FootEvent::Left;
		} else if (matches(tag, ".*Foot.*Right.*") && allow) {
			foot_kind = FootEvent::Right;
		} else if (matches(tag, ".*Foot.*Front.*") && allow) {
			foot_kind = FootEvent::Front;
		} else if (matches(tag, ".*Foot.*Back.*") && allow) {
			foot_kind = FootEvent::Back;
		} else if (matches(tag, ".*Jump.*(Down|Land).*")) {
			foot_kind = FootEvent::JumpLand;
		}
		return foot_kind;
	}

	std::vector<NiAVObject*> get_landing_nodes(Actor* actor, const FootEvent& foot_kind) {
		auto profiler = Profilers::Profile("Impact: Get Landing Nodes");
		std::vector<NiAVObject*> results;
		const std::string_view left_foot = "NPC L Foot [Lft ]";
		const std::string_view right_foot = "NPC R Foot [Rft ]";
		const std::string_view left_arm = "NPC L Hand [LHnd]";
		const std::string_view right_arm = "NPC R Hand [RHnd]";

		NiAVObject* result;
		switch (foot_kind) {
			case FootEvent::Left:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Right:
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Front:
				result = find_node(actor, left_arm);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_arm);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Back:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::JumpLand:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
		}
		return results;
	}
}
namespace Gts {
	ImpactManager& ImpactManager::GetSingleton() noexcept {
		static ImpactManager instance;
		return instance;
	}

	void ImpactManager::HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		if (a_event) {
			auto profiler = Profilers::Profile("Impact: HookProcess");
			auto actor = a_event->actor.get().get();
			
			std::string tag = a_event->tag.c_str();
			auto event_manager = ModEventManager::GetSingleton();
			event_manager.m_onfootstep.SendEvent(actor,tag);

			auto kind = get_foot_kind(actor, tag);
			Impact impact_data = Impact {
				.actor = actor,
				.kind = kind,
				.scale = get_visual_scale(actor),
				.nodes = get_landing_nodes(actor, kind),
			};

			EventDispatcher::DoOnImpact(impact_data); // Calls Explosions and sounds. A Must.

			float bonus = 1.0;
			if (actor->AsActorState()->IsWalking()) {
				bonus = 0.75;
			}
			if (actor->IsSneaking()) {
				bonus *= 0.5;
			}
			if (actor->AsActorState()->IsSprinting()) {
				bonus *= 1.15;
				if (Runtime::HasPerkTeam(actor, "LethalSprint")) {
					bonus *= 1.25;
				}
			}

			if (kind != FootEvent::JumpLand) {
				if (kind == FootEvent::Left) {
					DoDamageEffect(actor, Damage_Walk_Defaut, Radius_Walk_Default * bonus, 25, 0.25, kind, 1.25, DamageSource::WalkLeft);
				}
				if (kind == FootEvent::Right) {
					DoDamageEffect(actor, Damage_Walk_Defaut, Radius_Walk_Default * bonus, 25, 0.25, kind, 1.25, DamageSource::WalkRight);
				}
				//                     ^          ^
				//                 Damage         Radius
				DoLaunch(actor, 1.05 * bonus, 1.10 * bonus, kind);
				//               ^ radius      ^ push power
				return; // don't check further
			} else if (kind == FootEvent::JumpLand) {
				float perk = GetPerkBonus_Basics(actor);

				float fallmod = GetFallModifier(actor);
				auto& sizemanager = SizeManager::GetSingleton();
				
				float damage = sizemanager.GetSizeAttribute(actor, 2) * fallmod; // get jump damage boost

				float Start = Time::WorldTimeElapsed();
				ActorHandle gianthandle = actor->CreateRefHandle();
				std::string name = std::format("JumpLandT_{}", actor->formID);
				
				TaskManager::Run(name, [=](auto& progressData) { // Delay it a bit since it often happens in the air
					if (!gianthandle) {
						return false; // end task
					}
					auto giant = gianthandle.get().get();
					float timepassed = Time::WorldTimeElapsed() - Start;

					if (timepassed >= 0.15) {
						DoDamageEffect(giant, Damage_Jump_Default * damage, Radius_Jump_Default * fallmod, 20, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
						DoDamageEffect(giant, Damage_Jump_Default * damage, Radius_Jump_Default * fallmod, 20, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);

						DoLaunch(giant, 1.20 * perk * fallmod, 1.75 * fallmod, FootEvent::Left);
						DoLaunch(giant, 1.20 * perk * fallmod, 1.75 * fallmod, FootEvent::Right);
						return false;
					}
					return true;
				});
			}
		}
	}
}
