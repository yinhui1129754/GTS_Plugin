#include "managers/damage/SizeHitEffects.hpp"
#include "hooks/playerCharacter.hpp"
#include "managers/Attributes.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/plugin.hpp"
#include "events.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"
#include "utils/debug.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_PlayerCharacter::Hook() {
		log::info("Hooking PlayerCharacter");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCharacter[0] };
		// _GetBoundMin = Vtbl.write_vfunc(0x73, GetBoundMin);
		// _GetBoundMax = Vtbl.write_vfunc(0x74, GetBoundMax);
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);
		_SetSize = Vtbl.write_vfunc(REL::Relocate(0x0D9, 0x0D9, 0x0DB), SetSize);
		_Move = Vtbl.write_vfunc(REL::Relocate(0x0C8, 0x0C8, 0x0CA), Move);
		//_ProcessTracking = (Vtbl.write_vfunc(REL::Relocate(0x122, 0x122, 0x124), ProcessTracking));

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_PlayerCharacter[5] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetPermanentActorValue = Vtbl5.write_vfunc(0x02, GetPermanentActorValue);
		_GetBaseActorValue = Vtbl5.write_vfunc(0x03, GetBaseActorValue);
		_SetBaseActorValue = Vtbl5.write_vfunc(0x04, SetBaseActorValue);

		REL::Relocation<std::uintptr_t> AnimVtbl{ RE::VTABLE_PlayerCharacter[2] };
		_PCAnimEvents = AnimVtbl.write_vfunc(0x1, &PCAnimEvents);
	}

	void Hook_PlayerCharacter::HandleHealthDamage(PlayerCharacter* a_this, Actor* a_attacker, float a_damage) {
		if (a_attacker) {
			SizeHitEffects::GetSingleton().ApplyEverything(a_attacker, a_this, a_damage); // Apply bonus damage, overkill, stagger resistance
			if (Runtime::HasPerkTeam(a_this, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(a_this);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
			}
		}
		_HandleHealthDamage(a_this, a_attacker, a_damage);
	}

	void Hook_PlayerCharacter::AddPerk(PlayerCharacter* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerkEvent evt = AddPerkEvent {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoAddPerk(evt);
	}

	void Hook_PlayerCharacter::RemovePerk(PlayerCharacter* a_this, BGSPerk* a_perk) {
		RemovePerkEvent evt = RemovePerkEvent {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoRemovePerk(evt);
		_RemovePerk(a_this, a_perk);
	}

	float Hook_PlayerCharacter::GetActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight and Sneak
		float value = _GetActorValue(a_owner, a_akValue);
		if (Plugin::Ready()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				value = AttributeManager::AlterGetAv(a_this, a_akValue, value);
			}
		}
		return value;
	}

	float Hook_PlayerCharacter::GetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Health
		float value = _GetBaseActorValue(a_owner, a_akValue);
		if (Plugin::Ready()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			float bonus = 1.0;
			if (a_this) {
				value = AttributeManager::AlterGetBaseAv(a_this, a_akValue, value);
			}
		}
		return value;
	}

	void Hook_PlayerCharacter::SetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue, float value) {
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				value = AttributeManager::AlterSetBaseAv(a_this, a_akValue, value);
			}
		}
		_SetBaseActorValue(a_owner, a_akValue, value);
	}

	float Hook_PlayerCharacter::GetPermanentActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight and Damage
		float value = _GetPermanentActorValue(a_owner, a_akValue);
		if (Plugin::Ready()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				value = AttributeManager::AlterGetPermenantAv(a_this, a_akValue, value);
			}
		}
		return value;
	}

	void Hook_PlayerCharacter::SetSize(PlayerCharacter* a_this, float a_size) {
		// _SetSize(a_this, a_size);
	}

	void Hook_PlayerCharacter::Move(PlayerCharacter* a_this, float a_arg2, const NiPoint3& a_position) { // Override Movement Speed
		if (a_this->IsInKillMove()) {
			return _Move(a_this, a_arg2, a_position); // Do nothing in Kill moves
		}
		float bonus = AttributeManager::AlterMovementSpeed(a_this, a_position);
		return _Move(a_this, a_arg2, a_position * bonus);
	}

	void Hook_PlayerCharacter::ProcessTracking(PlayerCharacter* a_this, float a_delta, NiAVObject* a_obj3D) {
		float adjust = Runtime::GetFloat("ConversationCameraComp");
		auto player = PlayerCharacter::GetSingleton()->Get3D();
		_ProcessTracking(a_this, a_delta, a_obj3D);
	}

	void Hook_PlayerCharacter::PCAnimEvents(BSTEventSink<BSAnimationGraphEvent>* a_this, BSAnimationGraphEvent& a_event, BSTEventSource<BSAnimationGraphEvent>* a_src) {
		if (a_event.tag != nullptr && a_event.holder != nullptr) {
			Actor* actor = static_cast<Actor*>(a_this);
			if (actor) {
				EventDispatcher::DoActorAnimEvent(actor, a_event.tag, a_event.payload);
			}
		}
		return _PCAnimEvents(a_this, a_event, a_src);
	}

	NiPoint3 Hook_PlayerCharacter::GetBoundMax(PlayerCharacter* a_this) {
		auto bound = _GetBoundMax(a_this);
		if (a_this) {
			log::info("BoundMax: {}", Vector2Str(bound));
			float scale = get_giantess_scale(a_this);
			if (scale > 1e-4) {
				bound = bound * scale;
			}
			log::info("  - New BoundMax: {}", Vector2Str(bound));
		}
		return bound;
	}
	NiPoint3 Hook_PlayerCharacter::GetBoundMin(PlayerCharacter* a_this) {
		auto bound = _GetBoundMin(a_this);
		if (a_this) {
			log::info("BoundMin: {}", Vector2Str(bound));
			float scale = get_giantess_scale(a_this);
			if (scale > 1e-4) {
				bound = bound * scale;
			}
			log::info("  - BoundMin: {}", Vector2Str(bound));
		}
		return bound;
	}
}
