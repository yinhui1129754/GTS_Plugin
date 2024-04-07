#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/ai/headtracking.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "utils/MovementForce.hpp"
#include "utils/DynamicScale.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "scale/modscale.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "utils/camera.hpp"
#include "utils/debug.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void ManageActorControl() { // Rough control other fix
		auto profiler = Profilers::Profile("Manager: Actor Control");
		Actor* target = GetPlayerOrControlled();
		if (target->formID != 0x14) {
			auto grabbed = Grab::GetHeldActor(target);
			if (grabbed && grabbed->formID == 0x14) {
				return;
			}
			if (!IsGtsBusy(target) && !IsBeingHeld(target, PlayerCharacter::GetSingleton())) {
				ControlAnother(target, true);
			}
		}
	}


	void UpdateFalling() {
		Actor* player = PlayerCharacter::GetSingleton();
		if (player && player->IsInMidair()) {
			if (Runtime::HasPerkTeam(player, "MightyLegs")) {
				auto charCont = player->GetCharController();
				if (charCont) {
					auto transient = Transient::GetSingleton().GetData(player);
					if (transient) {
						float scale = std::clamp(get_visual_scale(player), 0.06f, 2.0f);
						float CalcFall = 1.0 + (charCont->fallTime * (4.0 / scale) - 4.0);
						float FallTime = std::clamp(CalcFall, 1.0f, 3.0f);
						transient->FallTimer = FallTime;
					}
				}
			}
		}
	}

	void FixActorFade(Actor* actor) {
		auto profiler = Profilers::Profile("Manager: Fade Fix");
		if (get_visual_scale(actor) < 1.5) {
			return;
		}
		if ((actor->formID == 0x14 || IsTeammate(actor))) {
			auto node = find_node(actor, "skeleton_female.nif");
			NiAVObject* skeleton = node;

			if (node) {
				BSFadeNode* fadenode = node->AsFadeNode();
				if (fadenode) {
					if (fadenode->GetRuntimeData().currentFade < 1.0f) {
						fadenode->GetRuntimeData().currentFade = 1.0f;
					}
				}
			}
		}
	}

	void update_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		auto profiler = Profilers::Profile("Manager: update_height");
		if (!actor) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float currentOtherScale = Get_Other_Scale(actor);
		trans_actor_data->otherScales = currentOtherScale;

		float target_scale = persi_actor_data->target_scale;
		
		// Smooth target_scale towards max_scale if target_scale > max_scale
		float max_scale = persi_actor_data->max_scale;
		if (target_scale > max_scale) {
			float minimum_scale_delta = 0.000005; // 0.00005%
			if (fabs(target_scale - max_scale) < minimum_scale_delta) {
				float target = max_scale;
				persi_actor_data->target_scale = target;
				persi_actor_data->target_scale_v = 0.0;
			} else {
				critically_damped(
					persi_actor_data->target_scale,
					persi_actor_data->target_scale_v,
					max_scale,
					persi_actor_data->half_life*1.5,
					Time::WorldTimeDelta()
					);
			}
		} else {
			persi_actor_data->target_scale_v = 0.0;
		}

		// Room Size adjustments
		// We only do this if they are bigger than 1.05x their natural scale (currentOtherScale)
		// and if enabled in the mcm
		if (SizeRaycastEnabled() && !actor->IsDead() && target_scale > 1.025) {
			float room_scale = GetMaxRoomScale(actor);
			if (room_scale > (currentOtherScale - 0.05)) {
				// Only apply room scale if room_scale > natural_scale
				//   This stops it from working when room_scale < 1.0
				//if (actor->formID == 0x14) {
					//log::info("old target_scale: {}", target_scale);
					//log::info("room_scale: {}", room_scale);
				//}
				target_scale = min(target_scale, room_scale);
				//if (actor->formID == 0x14) {
					//log::info("new target_scale: {}", target_scale);
				//}
			} else {
				// Else we just scale to natural
				target_scale = 1.0;
			}
		}

		if (fabs(target_scale - persi_actor_data->visual_scale) > 1e-5) {
			float minimum_scale_delta = 0.000005; // 0.00005%
			if (fabs(target_scale - persi_actor_data->visual_scale) < minimum_scale_delta) {
				persi_actor_data->visual_scale = target_scale;
				persi_actor_data->visual_scale_v = 0.0;
			} else {
				critically_damped(
					persi_actor_data->visual_scale,
					persi_actor_data->visual_scale_v,
					target_scale,
					persi_actor_data->half_life / TimeScale(),
					Time::WorldTimeDelta()
					);
			}
		}
	}
	void apply_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		auto profiler = Profilers::Profile("Manager: apply_height");
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float scale = get_scale(actor);
		if (scale < 0.0) {
			return;
		}
		float visual_scale = persi_actor_data->visual_scale;
		if (actor->formID == 0x14) {
			float scaleOverride = get_fp_scale(actor);
			if (IsFirstPerson() && scaleOverride >= 1e-4) {
				if (scaleOverride > 1.0) {
					visual_scale *= GetProneAdjustment(); // In normal case we * it for compatibility with crawling/proning.
				} else {
					visual_scale = scaleOverride; // In Loot/Combat mode case, we override it with flat value (such as 0.6).
				}
			}
		}

		// Is scale correct already?
		if (fabs(visual_scale - scale) <= 1e-5 && !force) {
			return;
		}
		// Is scale too small
		if (visual_scale <= 1e-5) {
			return;
		}

		float initialScale = GetInitialScale(actor); // Incorperate the NIF scale into our edits
		float GameScale = game_getactorscale(actor); // * by GetScale
		
		update_model_visuals(actor, visual_scale * initialScale * GameScale); // We've set the values, now update model size based on them
	}

	void apply_speed(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		auto profiler = Profilers::Profile("Manager: apply_speed");
		if (!Persistent::GetSingleton().is_speed_adjusted) {
			return;
		}
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float scale = get_visual_scale(actor);
		if (scale < 1e-5) {
			return;
		}
		float speedmultcalc = GetAnimationSlowdown(actor); // For all other movement types
		float perkspeed = 1.0;
		persi_actor_data->anim_speed = speedmultcalc*perkspeed;//MS_mult;
	}

	/*void update_effective_multi(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		auto profiler = Profilers::Profile("Manager: update_effective_multi");
		if (!actor) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		if (HasSMT(actor)) {
			persi_actor_data->effective_multi = 2.0;
		} else {
			persi_actor_data->effective_multi = 1.0;
		}
	}*/

	void update_actor(Actor* actor) {
		auto profiler = Profilers::Profile("Manager: update_actor");
		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		//update_effective_multi(actor, saved_data, temp_data);
		update_height(actor, saved_data, temp_data);
	}

	void apply_actor(Actor* actor, bool force = false) {
		auto profiler = Profilers::Profile("Manager: apply_actor");
		auto temp_data = Transient::GetSingleton().GetData(actor);
		auto saved_data = Persistent::GetSingleton().GetData(actor);
		apply_height(actor, saved_data, temp_data, force);
		apply_speed(actor, saved_data, temp_data, force);
	}
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;

	static std::atomic_bool initialized;
	static std::latch latch(1);
	if (!initialized.exchange(true)) {
		latch.count_down();
	}
	latch.wait();

	return instance;
}

std::string GtsManager::DebugName() {
	return "GtsManager";
}

void GtsManager::Start() {
}

// Poll for updates
void GtsManager::Update() {
	auto profiler = Profilers::Profile("Manager: Update()");

	UpdateFalling();
	ManageActorControl(); // Sadly have to call it non stop since im unsure how to easily fix it otherwise :(

	for (auto actor: find_actors()) {
		if (!actor) {
			return; 
		}

		FixActorFade(actor);
		
		auto& CollisionDamage = CollisionDamage::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		if (actor->formID == 0x14 || IsTeammate(actor)) {
			
			ScareActors(actor);

			CollisionDamage.DoFootCollision(actor, Damage_Default_Underfoot * TimeScale(), Radius_Default_Idle, 4000, 0.05, Minimum_Actor_Crush_Scale_Idle, DamageSource::CrushedLeft, false, false);
			CollisionDamage.DoFootCollision(actor, Damage_Default_Underfoot * TimeScale(), Radius_Default_Idle, 4000, 0.05, Minimum_Actor_Crush_Scale_Idle, DamageSource::CrushedRight, true, false);
			
			ClothManager::GetSingleton().CheckRip();
			TinyCalamity_SeekActors(actor);
			SpawnActionIcon(actor);

			if (IsCrawling(actor)) {
				ApplyAllCrawlingDamage(actor, 1000, 0.25);
			}

			GameModeManager::GetSingleton().GameMode(actor); // Handle Game Modes
		}
		if (Runtime::GetBool("PreciseDamageOthers")) {
			if (actor->formID != 0x14 && !IsTeammate(actor)) {
				CollisionDamage.DoFootCollision(actor, Damage_Default_Underfoot * TimeScale(), Radius_Default_Idle, 4000, 0.05, Minimum_Actor_Crush_Scale_Idle, DamageSource::CrushedLeft, false, false);
				CollisionDamage.DoFootCollision(actor, Damage_Default_Underfoot * TimeScale(), Radius_Default_Idle, 4000, 0.05, Minimum_Actor_Crush_Scale_Idle, DamageSource::CrushedRight, true, false);
			}
		}
		update_actor(actor);
		apply_actor(actor);
	}
}

void GtsManager::OnAddPerk(const AddPerkEvent& evt) {
	if (evt.actor->formID == 0x14) {
		if (evt.perk == Runtime::GetPerk("ColossalGrowth")) {
			CallHelpMessage();
		}
		if (evt.perk == Runtime::GetPerk("FastShrink") && !Runtime::HasSpell(evt.actor, "ShrinkBolt")) {
			Runtime::AddSpell(evt.actor, "ShrinkBolt");
		}
		if (evt.perk == Runtime::GetPerk("LethalShrink") && !Runtime::HasSpell(evt.actor, "ShrinkStorm")) {
			Runtime::AddSpell(evt.actor, "ShrinkStorm");
		}
		if (evt.perk == Runtime::GetPerk("CalamityPerk")) {
			AddCalamityPerk();
		}
	}
}

void GtsManager::DragonSoulAbsorption() {
	DragonAbsorptionBonuses(); 
}

void GtsManager::reapply(bool force) {
	// Get everyone in loaded AI data and reapply
	auto profiler = Profilers::Profile("Manager: reapply");
	auto actors = find_actors();
	for (auto actor: actors) {
		if (!actor) {
			continue;
		}
		if (!actor->Is3DLoaded()) {
			continue;
		}
		reapply_actor(actor, force);
	}
}
void GtsManager::reapply_actor(Actor* actor, bool force) {
	auto profiler = Profilers::Profile("Manager: reapply_actor");
	// Reapply just this actor
	if (!actor) {
		return;
	}
	if (!actor->Is3DLoaded()) {
		return;
	}
	apply_actor(actor, force);
}