#include "hooks/magicTarget.hpp"
#include "data/persistent.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_MagicTarget::Hook() {
		log::info("Hooking ActorTarget");

		REL::Relocation<std::uintptr_t> playerVtbl{ PlayerCharacter::VTABLE[1] };
		_AddTargetPlayer = playerVtbl.write_vfunc(0x01, AddTargetPlayer);

		REL::Relocation<std::uintptr_t> characterVtbl{ Character::VTABLE[1] };
		_AddTargetPlayer = characterVtbl.write_vfunc(0x01, AddTargetCharacter);
	}

	bool Hook_MagicTarget::AddTargetPlayer(PlayerCharacter* player, MagicTarget::AddTargetData& a_targetData) {
		log::info("Add magic target player");
		auto effect = a_targetData.effect;
		if (effect) {
			log::info("  - Has effect");
			auto baseEffect = effect->baseEffect;
			if (baseEffect) {
				log::info("    - Has base effect: {}", baseEffect->GetFullName());
			}
		}
		return _AddTargetPlayer(player, a_targetData);
	}

	bool Hook_MagicTarget::AddTargetCharacter(Character* character, MagicTarget::AddTargetData& a_targetData) {
		log::info("Add magic target character");
		auto effect = a_targetData.effect;
		if (effect) {
			log::info("  - Has effect");
			auto baseEffect = effect->baseEffect;
			if (baseEffect) {
				log::info("    - Has base effect: {}", baseEffect->GetFullName());
			}
		}
		return _AddTargetCharacter(character, a_targetData);
	}
}
