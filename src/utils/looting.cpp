#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/animation/Grab.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "utils/DeathReport.hpp"
#include "managers/highheel.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/looting.hpp"
#include "managers/Rumble.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "spring.hpp"
#include "scale/scale.hpp"
#include "colliders/RE.hpp"
#include "colliders/actor.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"
#include "utils/av.hpp"
#include "colliders/RE.hpp"
#include "UI/DebugAPI.hpp"
#include "rays/raycast.hpp"
#include <vector>
#include <string>


using namespace RE;
using namespace Gts;

namespace {
	void RunScaleTask(ObjectRefHandle dropboxHandle, Actor* actor, const float Start, const float Scale, const bool soul, const NiPoint3 TotalPos) {
		std::string taskname = std::format("Dropbox {}", actor->formID); // create task name for main task
		TaskManager::RunFor(taskname, 16, [=](auto& progressData) { // Spawn loot piles
			if (!dropboxHandle) {
				return false;
			}
			float Finish = Time::WorldTimeElapsed();
			auto dropboxPtr = dropboxHandle.get().get();
			if (!dropboxPtr->Is3DLoaded()) {
				return true;
			}
			auto dropbox3D = dropboxPtr->GetCurrent3D();
			if (!dropbox3D) {
				return true; // Retry next frame
			} else {
				float timepassed = Finish - Start;
				if (soul) {
					timepassed *= 1.33; // faster soul scale
				}
				auto node = find_object_node(dropboxPtr, "GorePile_Obj");
				auto trigger = find_object_node(dropboxPtr, "Trigger_Obj");
				if (node) {
					node->local.scale = (Scale * 0.33) + (timepassed*0.18);
					if (!soul) {
						node->world.translate.z = TotalPos.z;
					}
					update_node(node);
				}
				if (trigger) {
					trigger->local.scale = (Scale * 0.33) + (timepassed*0.18);
					if (!soul) {
						trigger->world.translate.z = TotalPos.z;
					}
					update_node(trigger);
				}
				if (node && node->local.scale >= Scale) { // disable collision once it is scaled enough
					return false; // End task
				}
				return true;
			}
		});
	}

	void RunAudioTask(ObjectRefHandle dropboxHandle, Actor* actor) {
		std::string taskname_sound = std::format("DropboxAudio {}", actor->formID);
		TaskManager::RunFor(taskname_sound, 6, [=](auto& progressData) {
			if (!dropboxHandle) {
				return false;
			}
			auto dropboxPtr = dropboxHandle.get().get();
			if (!dropboxPtr->Is3DLoaded()) {
				return true; // retry
			}
			auto dropbox3D = dropboxPtr->GetCurrent3D();
			if (!dropbox3D) {
				return true; // Retry next frame
			} else {
				Runtime::PlaySound("GtsCrushSound", dropboxPtr, 1.0, 1.0);
				return false;
			}
		});
	}
}

namespace Gts {

	NiPoint3 GetContainerSpawnLocation(Actor* giant, Actor* tiny) {
		bool success_first = false;
		bool success_second = false;
		NiPoint3 ray_start = tiny->GetPosition();
		ray_start.z += 40.0; // overrize .z with giant .z + 40, so ray starts from above
		NiPoint3 ray_direction(0.0, 0.0, -1.0);

		float ray_length = 40000;

		NiPoint3 pos = NiPoint3(0, 0, 0); // default pos
		NiPoint3 endpos = CastRayStatics(tiny, ray_start, ray_direction, ray_length, success_first);
		if (success_first) {
			return endpos;
		} else if (!success_first) {
			NiPoint3 ray_start_second = giant->GetPosition();
			ray_start_second.z += 40.0;
			pos = CastRayStatics(giant, ray_start_second, ray_direction, ray_length, success_second);
			if (!success_second) {
				pos = giant->GetPosition();
				return pos;
			}
			return pos;
		}
		return pos;
	}

	void TransferInventory(Actor* from, Actor* to, const float scale, bool keepOwnership, bool removeQuestItems, DamageSource Cause, bool reset) {
		std::string name = std::format("TransferItems_{}_{}", from->formID, to->formID);

		bool reanimated = false; // shall we avoid transfering items or not.
		if (Cause != DamageSource::Vored) {
			reanimated = WasReanimated(from);
		}
		if (Runtime::IsRace(from, "IcewraithRace")) {
			reanimated = true;
		}
		// ^ we generally do not want to transfer loot in that case: 2 loot piles will spawn if actor was resurrected

		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = to->CreateRefHandle();
		ActorHandle tinyhandle = from->CreateRefHandle();
		bool PCLoot = Runtime::GetBool("GtsEnableLooting");
		bool NPCLoot = Runtime::GetBool("GtsNPCEnableLooting");

		float expectedtime = 0.1;
		if (IsDragon(from)) {
			expectedtime = 0.25; // Because dragons don't spawn loot right away...sigh...
		}

		if (reset) {
			StartResetTask(from); // reset actor data.
		}

		TaskManager::RunFor(name, 3.0, [=](auto& progressData) {
			if (!tinyhandle) {
				return false;
			}
			if (!gianthandle) {
				return false;
			}
			auto tiny = tinyhandle.get().get();
			auto giant = gianthandle.get().get();

			float hp = GetAV(tiny, ActorValue::kHealth);

			if (!tiny->IsDead()) {
				KillActor(giant, tiny); // just to make sure
			}
			if (tiny->IsDead() || hp <= 0.0) {
				float Finish = Time::WorldTimeElapsed();
				float timepassed = Finish - Start;
				if (timepassed < expectedtime) {
					return true; // retry, not enough time has passed yet
				}
				TESObjectREFR* ref = skyrim_cast<TESObjectREFR*>(tiny);

				if (giant->formID == 0x14 && !PCLoot) {
					TransferInventory_Normal(giant, tiny, removeQuestItems);
					return false;
				}
				if (giant->formID != 0x14 && !NPCLoot) {
					TransferInventory_Normal(giant, tiny, removeQuestItems);
					return false;
				}
				TransferInventoryToDropbox(giant, tiny, scale, removeQuestItems, Cause, reanimated);
				return false; // stop it, we started the looting of the Target.
			}
			return true;
		});
	}

	void TransferInventory_Normal(Actor* giant, Actor* tiny, bool removeQuestItems) {
		int32_t quantity = 1.0;

		for (auto &[a_object, invData]: tiny->GetInventory()) { // transfer loot
			if (a_object->GetPlayable() && a_object->GetFormType() != FormType::LeveledItem) {
				if ((!invData.second->IsQuestObject() || removeQuestItems)) {

					TESObjectREFR* ref = skyrim_cast<TESObjectREFR*>(tiny);
					if (ref) {
						auto changes = ref->GetInventoryChanges();
						if (changes) {
							quantity = GetItemCount(changes, a_object); // obtain item count
						}
					}


					tiny->RemoveItem(a_object, quantity, ITEM_REMOVE_REASON::kRemove, nullptr, giant, nullptr, nullptr);
				}
			}
		}
	}



	void TransferInventoryToDropbox(Actor* giant, Actor* actor, const float scale, bool removeQuestItems, DamageSource Cause, bool Resurrected) {
		bool soul = false;
		float Scale = std::clamp(scale * GetSizeFromBoundingBox(actor), 0.10f, 4.4f);

		if (Resurrected) {
			return;
		}

		std::string_view container;
		std::string name = std::format("{} remains", actor->GetDisplayFullName());

		if (IsMechanical(actor)) {
			container = "Dropbox_Mechanical";
		} else if (Cause == DamageSource::Vored) { // Always spawn soul on vore
			container = "Dropbox_Soul";
			name = std::format("{} Soul Remains", actor->GetDisplayFullName());
			soul = true;
		} else if (LessGore()) { // Always Spawn soul if Less Gore is on
			container = "Dropbox_Soul";
			name = std::format("Crushed Soul of {} ", actor->GetDisplayFullName());
			soul = true;
		} else if (IsInsect(actor, false)) {
			container = "Dropbox_Bug";
			name = std::format("Remains of {}", actor->GetDisplayFullName());
		} else if (IsLiving(actor)) {
			container = "Dropbox"; // spawn normal dropbox
		} else {
			container = "Dropbox_Undead";
		}



		NiPoint3 TotalPos = GetContainerSpawnLocation(giant, actor); // obtain goal of container position by doing ray-cast
		if (IsDebugEnabled()) {
			DebugAPI::DrawSphere(glm::vec3(TotalPos.x, TotalPos.y, TotalPos.z), 8.0, 6000, {1.0, 1.0, 0.0, 1.0});
		}
		auto dropbox = Runtime::PlaceContainerAtPos(actor, TotalPos, container); // Place chosen container

		if (!dropbox) {
			return;
		}
		float Start = Time::WorldTimeElapsed();
		dropbox->SetDisplayName(name, false); // Rename container to match chosen name
		if (IsDragon(actor) && !actor->IsDead()) {
			TESObjectREFR* ref = skyrim_cast<TESObjectREFR*>(actor);
			ref->GetInventoryChanges()->InitLeveledItems();
			log::info("{} Isn't dead, initiating leveled items", actor->GetDisplayFullName());
		}

		ObjectRefHandle dropboxHandle = dropbox->CreateRefHandle();

		if (Cause == DamageSource::Overkill) { // Play audio that won't disappear if source of loot transfer is Overkill
			RunAudioTask(dropboxHandle, actor); // play sound
		}
		if (dropboxHandle) {
			float scale_up = std::clamp(Scale, 0.10f, 1.0f);
			TotalPos.z += (200.0 - (200.0 * scale_up)); // move it a bit upwards
			RunScaleTask(dropboxHandle, actor, Start, Scale, soul, TotalPos); // Scale our pile over time
		}
		MoveItemsTowardsDropbox(actor, dropbox, removeQuestItems); // Launch transfer items task with a bit of delay
	}

	void MoveItemsTowardsDropbox(Actor* actor, TESObjectREFR* dropbox, bool removeQuestItems) {
		int32_t quantity = 1.0;
		for (auto &[a_object, invData]: actor->GetInventory()) { // transfer loot
			if (a_object->GetPlayable() && a_object->GetFormType() != FormType::LeveledItem) { // We don't want to move Leveled Items
				if ((!invData.second->IsQuestObject() || removeQuestItems)) {

					TESObjectREFR* ref = skyrim_cast<TESObjectREFR*>(actor);
					if (ref) {
						//log::info("Transfering item: {}, looking for quantity", a_object->GetName());
						auto changes = ref->GetInventoryChanges();
						if (changes) {
							quantity = GetItemCount(changes, a_object); // obtain item count
						}
					}

					//log::info("Transfering item: {}, quantity: {}", a_object->GetName(), quantity);

					actor->RemoveItem(a_object, quantity, ITEM_REMOVE_REASON::kRemove, nullptr, dropbox, nullptr, nullptr);
				}
			}
		}
	}
}