#include "managers/impact.hpp"
#include "hooks/impact.hpp"
#include "scale/scale.hpp"


using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_BGSImpactManager::Hook(Trampoline& trampoline) {
		static FunctionHook<bool(BGSImpactManager* manager, BGSImpactManager::ImpactSoundData& data)>PlayImpactDataSounds(
			REL::RelocationID(35317, 36212),
			[](BGSImpactManager* manager, auto& data) {
				auto& Object = data.objectToFollow;
				// Trying to get/alter data.sound1/sound2 is useless since they never pass the if (sound) check
				// So the only way to disable default sounds is to do data.playsound1/2 = false
				if (Object) {
					auto Data = Object->GetUserData();
					if (Data) { // GetUserData seems to be player exclusive for some reason
						Actor* actor = skyrim_cast<Actor*>(Data);
						if (actor) {
							if (actor->formID == 0x14 && get_visual_scale(actor) >= 1.75) {
								//log::info("Disabling sounds for {}", actor->GetDisplayFullName());
								data.playSound1 = false;
								data.playSound2 = false;
							}
						}
					}
				}
				return PlayImpactDataSounds(manager, data);
			}
		);

		/*static FunctionHook<void(BSSoundHandle* handle, NiAVObject* node)>SetObjectToFollow(
			REL::RelocationID(Offset::BSSoundHandle::SetObjectToFollow),
			[](BSSoundHandle* handle, auto* node) {
				log::info("ObjectToFollow hooked");
				// This method works and seems to report audio for everyone
				// but need to figure out how to disable exactly what we want (footsteps)
				
				if (node) {
					auto Data = node->GetUserData();
					if (Data) { // GetUserData seems to be player exclusive for some reason
						
						Actor* actor = skyrim_cast<Actor*>(Data);
						if (actor && actor->formID == 0x14) {
							log::info("UserData found");
							log::info("Sound ID: {}", handle->soundID); // each next sound is ID + 1 or 2 so it's impossible to whitelist by ID...
							log::info("Pad 05: {}", handle->pad05); // always 0
							log::info("Pad 06: {}", handle->pad06); // always 0
							if (get_visual_scale(actor) >= 1.5) {
								log::info("Disabling sounds for {}", actor->GetDisplayFullName());
								//data.playSound1 = false;
								//data.playSound2 = false;
							}
						}
					}
				}
				return SetObjectToFollow(handle, node);
			}
		);*/
	}

	void Hook_BGSImpactManager::Hook() {
		log::info("Hooking BGSImpactManager");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BGSImpactManager[0] };
		_ProcessEvent = Vtbl.write_vfunc(0x01, ProcessEvent);
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		Gts::ImpactManager::GetSingleton().HookProcessEvent(a_this, a_event, a_eventSource); 
		// ^ On FootEvent: manages damage, effects and launching. do NOT disable it!
		auto result = _ProcessEvent(a_this, a_event, a_eventSource);
		return result;
	}
}
