#include "hooks/hooks.hpp"
#include "hooks/main.hpp"
#include "hooks/headTracking.hpp"
#include "hooks/actorRotation.hpp"
#include "hooks/PreventAnimations.hpp"
#include "hooks/controls.hpp"
#include "hooks/Stealth.hpp"
#include "hooks/impact.hpp"
#include "hooks/Pushback.hpp"
#include "hooks/vm.hpp"
#include "hooks/havok.hpp"
#include "hooks/magicTarget.hpp"
#include "hooks/hkbBehaviorGraph.hpp"
#include "hooks/cameraState.hpp"
#include "hooks/playerCamera.hpp"
#include "hooks/Movement.hpp"
#include "hooks/playerCharacter.hpp"
#include "hooks/actor.hpp"
#include "hooks/character.hpp"
#include "hooks/sink.hpp"
#include "hooks/jump.hpp"
#include "hooks/Experiments.hpp"
#include "hooks/damage.hpp"

using namespace RE;

namespace Hooks
{
	void InstallControls() {
		log::info("Applying Control Hooks...");
		Hook_Controls<ActivateHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ActivateHandler[0]));
		Hook_Controls<AttackBlockHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_AttackBlockHandler[0]));
		Hook_Controls<AutoMoveHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_AutoMoveHandler[0]));
		Hook_Controls<JumpHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]));
		Hook_Controls<MovementHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_MovementHandler[0]));
		Hook_Controls<ReadyWeaponHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ReadyWeaponHandler[0]));
		Hook_Controls<RunHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_RunHandler[0]));
		Hook_Controls<ShoutHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ShoutHandler[0]));
		Hook_Controls<SneakHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_SneakHandler[0]));
		//Hook_Controls<ThirdPersonState>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ThirdPersonState[0]));
		Hook_Controls<SprintHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_SprintHandler[0]));
		Hook_Controls<ToggleRunHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ToggleRunHandler[0]));
		log::info("Gts finished applying Control hooks...");
	}

	void Install()
	{
		log::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(512);

		Hook_MainUpdate::Hook(trampoline);
		Hook_BGSImpactManager::Hook();
		Hook_VM::Hook();
		Hook_Havok::Hook(trampoline);
		//Hook_MagicTarget::Hook();
		Hook_hkbBehaviorGraph::Hook();
		Hook_PlayerCharacter::Hook();
		Hook_Actor::Hook(trampoline);
		Hook_Character::Hook();
		Hook_Sinking::Hook(trampoline);
		Hook_Jumping::Hook(trampoline);
		Hook_Damage::Hook(trampoline);
		Hook_Pushback::Hook(trampoline);

		InstallControls();
		//if (REL::Module::IsSE()) { // Used when something is not RE'd yet for AE
			
		//}
		Hook_Stealth::Hook(trampoline);
		Hook_Movement::Hook(trampoline);
		//Hook_Experiments::Hook(trampoline);
		Hook_HeadTracking::Hook(trampoline);
		Hook_PreventAnimations::Hook(trampoline);
		//Hook_ActorRotation::Hook(trampoline);
		HookCameraStates();

		log::info("Gts finished applying hooks...");
	}
}