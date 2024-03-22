#pragma once
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/hitmanager.hpp"
#include "managers/Attributes.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/plugin.hpp"
#include "events.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;

namespace {
    bool IsGtsBusy_ForControls(Actor* actor) {
		bool GTSBusy = false;
		actor->GetGraphVariableBool("GTS_Busy", GTSBusy);
        // Have to use this because Hand Swipes make original bool return false

		return GTSBusy;
	}

	bool IsGrabAttacking(Actor* actor) {
		bool Attacking = false;
		actor->GetGraphVariableBool("GTS_IsGrabAttacking", Attacking);

		return Attacking;
	}

    bool CanMove() {
        auto player = PlayerCharacter::GetSingleton();
        if (!player) {
            return true;
        }
		Actor* Controlled = GetPlayerOrControlled();
		if (Controlled ->formID != 0x14) {
			if (IsThighSandwiching(Controlled)) { // Disallow player movement if we have control over other actor and actor does thigh sandwich
				return false;
			} if (IsBetweenBreasts(Controlled)) {
				return false;
			}
		}
		if (!AnimationsInstalled(player)) { // Don't mess with movement if user didn't install anims correctly
			return true;
		}
		if (IsTransitioning(player)) { // Disallow to move during transition
			return false;
		}
		if (IsGrabAttacking(player)) { // Allow movement for Grab Attacking
			return true;
		}
        return !IsGtsBusy_ForControls(player); // Else return GTS Busy
    }
}

namespace Hooks
{   
	template <class T>
	class Hook_Controls: public T { // HUGE Credits to Vermunds (SkyrimSoulsRE source code)!
		
	public:
		using CanProcess_t = decltype(&T::CanProcess);
		static inline REL::Relocation<CanProcess_t> _CanProcess;  

		bool HookMovement(RE::InputEvent* a_event);

		static void Hook(REL::Relocation<std::uintptr_t> a_vtbl);
	};

	template <class T>
	inline bool Hook_Controls<T>::HookMovement(RE::InputEvent* a_event) {
		if (!CanMove()) {
			//log::info("Movement Disabled");
			return false;
		}
		return _CanProcess(this, a_event);
	}

	template <class T>
	inline void Hook_Controls<T>::Hook(REL::Relocation<std::uintptr_t> a_vtbl) {
		_CanProcess = a_vtbl.write_vfunc(0x1, &HookMovement);
	}
}
