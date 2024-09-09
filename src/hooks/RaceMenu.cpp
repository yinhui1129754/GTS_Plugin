
#include "utils/actorUtils.hpp"
#include "hooks/RaceMenu.hpp"
#include "hooks/callhook.hpp"
#include "data/transient.hpp"
#include "scale/modscale.hpp"
#include "data/runtime.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"


using namespace RE;
using namespace SKSE;

namespace Hooks {
    void Hook_RaceMenu::Hook(Trampoline& trampoline) {
        static FunctionHook<void(Actor* actor, TESRace* a_race, bool a_player)>SwitchRaceHook(
            REL::RelocationID(36901, 37925),
            [](Actor* actor, TESRace* a_race, bool a_player) {
                if (actor && actor->formID == 0x14) { // Updates natural scale of Player when changing races
                    log::info("SwitchRace hooked!");
                    RefreshInitialScales(actor);
                }
                    
                return SwitchRaceHook(actor, a_race, a_player);
            }
        );
    }
}