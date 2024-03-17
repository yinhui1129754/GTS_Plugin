#include "hooks/actor.hpp"
#include "managers/Attributes.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/plugin.hpp"
#include "events.hpp"

using namespace RE;
using namespace Gts;

namespace {
	float GetActorValueModifier(float original_value, Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		if (Plugin::InGame()) {
			if (a_this) {
				//log::info("AttributeManager::AlterGetAvMod");
				return AttributeManager::AlterGetAvMod(original_value, a_this, a_modifier, a_value);
			}
		}
		return original_value;
	}
}

namespace Hooks
{
	void Hook_Actor::Hook(Trampoline& trampoline) {
		log::info("Hooking Actor");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_Actor[0] };
		// _GetBoundMin = Vtbl.write_vfunc(0x73, GetBoundMin);
		// _GetBoundMax = Vtbl.write_vfunc(0x74, GetBoundMax);
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);
		_Move = Vtbl.write_vfunc(REL::Relocate(0x0C8, 0x0C8, 0x0CA), Move);

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_Actor[5] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetPermanentActorValue = Vtbl5.write_vfunc(0x02, GetPermanentActorValue);
		_GetBaseActorValue = Vtbl5.write_vfunc(0x03, GetBaseActorValue);
		_SetBaseActorValue = Vtbl5.write_vfunc(0x04, SetBaseActorValue);

		//REL::Relocation<uintptr_t*> getavmod1(REL::ID(36350), REL::Offset(0x22));
		//REL::Relocation<uintptr_t*> getavmod2(REL::ID(37513), REL::Offset(0x2d));
		//REL::Relocation<uintptr_t*> getavmod3(REL::ID(37537), REL::Offset(0x6b));
		//REL::Relocation<uintptr_t*> getavmod4(REL::ID(37537), REL::Offset(0x7f));
		//REL::Relocation<uintptr_t*> getavmod5(REL::ID(37539), REL::Offset(0x4a));
		//REL::Relocation<uintptr_t*> getavmod6(REL::ID(51473), REL::Offset(0x3d9));
		//REL::Relocation<uintptr_t*> getavmod7(REL::ID(51473), REL::Offset(0x543));
		//REL::Relocation<uintptr_t*> getavmod8(REL::ID(51473), REL::Offset(0x6ad));
		//REL::Relocation<uintptr_t*> getavmod9(REL::ID(52059), REL::Offset(0x59));
		//REL::Relocation<uintptr_t*> getavmod1(REL::ID(22450), REL::Offset(0x27d));
		//REL::Relocation<uintptr_t*> getavmod2(REL::ID(37537), REL::Offset(0x6b));
		//REL::Relocation<uintptr_t*> getavmod3(REL::ID(37539), REL::Offset(0x4a));
		//REL::Relocation<uintptr_t*> getavmod4(REL::ID(53867), REL::Offset(0x72));

		//_GetActorValueModifier_1 = trampoline.write_call<5>(getavmod1.address(), GetActorValueModifier_1);
		//_GetActorValueModifier_2 = trampoline.write_call<5>(getavmod2.address(), GetActorValueModifier_2);
		//_GetActorValueModifier_3 = trampoline.write_call<5>(getavmod3.address(), GetActorValueModifier_3);
		//_GetActorValueModifier_4 = trampoline.write_call<5>(getavmod4.address(), GetActorValueModifier_4);
		// _GetActorValueModifier_5 = trampoline.write_call<5>(getavmod5.address(), GetActorValueModifier_5);
		//_GetActorValueModifier_6 = trampoline.write_call<5>(getavmod6.address(), GetActorValueModifier_6);
		//_GetActorValueModifier_7 = trampoline.write_call<5>(getavmod7.address(), GetActorValueModifier_7);
		//_GetActorValueModifier_8 = trampoline.write_call<5>(getavmod8.address(), GetActorValueModifier_8);
		//_GetActorValueModifier_9 = trampoline.write_call<5>(getavmod9.address(), GetActorValueModifier_9);
	}

	void Hook_Actor::HandleHealthDamage(Actor* a_this, Actor* a_attacker, float a_damage) {
		if (a_attacker) {
			if (Runtime::HasPerkTeam(a_this, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(a_this);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
			}
		}
		_HandleHealthDamage(a_this, a_attacker, a_damage);  // Just reports the value, can't override it.
	}

	void Hook_Actor::AddPerk(Actor* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerkEvent evt = AddPerkEvent {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoAddPerk(evt);
	}

	void Hook_Actor::RemovePerk(Actor* a_this, BGSPerk* a_perk) {
		RemovePerkEvent evt = RemovePerkEvent {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoRemovePerk(evt);
		_RemovePerk(a_this, a_perk);
	}

	float Hook_Actor::GetActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight
		float value = _GetActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					value = AttributeManager::AlterGetAv(a_this, a_akValue, value);
				}
			}
		}
		return value;
	}

	float Hook_Actor::GetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Health
		float value = _GetBaseActorValue(a_owner, a_akValue);
		float bonus = 0.0;
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					bonus = AttributeManager::AlterGetBaseAv(a_this, a_akValue, value);
				}
			}
		}
		return value + bonus;
	}

	void Hook_Actor::SetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue, float value) {
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					value = AttributeManager::AlterSetBaseAv(a_this, a_akValue, value);
				}
			}
		}
		_SetBaseActorValue(a_owner, a_akValue, value);
	}

	float Hook_Actor::GetPermanentActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight and Damage
		float value = _GetPermanentActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					value = AttributeManager::AlterGetPermenantAv(a_this, a_akValue, value);
				}
			}
		}
		return value;
	}

	void Hook_Actor::Move(Actor* a_this, float a_arg2, const NiPoint3& a_position) { // Override Movement Speed
		if (a_this->IsInKillMove()) {
			return _Move(a_this, a_arg2, a_position); // Do nothing in Kill moves
		}
		float bonus = AttributeManager::AlterMovementSpeed(a_this, a_position);
		return _Move(a_this, a_arg2, a_position * bonus);
	}

	float Hook_Actor::GetActorValueModifier_1(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_1(a_this, a_modifier, a_value);
		float bonus = 0.0;
		if (Plugin::InGame() && a_this) {
			if (a_this->formID == 0x14 && a_value == ActorValue::kHealth) {
				float value = (a_this->AsActorValueOwner()->GetBaseActorValue(ActorValue::kHealth)) - original_value;
				bonus = AttributeManager::AlterGetBaseAv(a_this, a_value, value);
				//log::info("GetAV1 true, modifier {}, value: {}, original_value: {}, bonus: {}", static_cast<uint32_t>(a_modifier), static_cast<uint32_t>(a_value), original_value, bonus);
			}
		}
		return GetActorValueModifier(original_value + bonus, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_2(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_1(a_this, a_modifier, a_value);
		float bonus = 0.0;
		if (Plugin::InGame() && a_this) {
			if (a_this->formID == 0x14 && a_value == ActorValue::kHealth) {
				float value = (a_this->AsActorValueOwner()->GetBaseActorValue(ActorValue::kHealth)) - original_value;
				bonus = AttributeManager::AlterGetBaseAv(a_this, a_value, value);
				//log::info("GetAV2 true, modifier {}, value: {}, original_value: {}, bonus: {}", static_cast<uint32_t>(a_modifier), static_cast<uint32_t>(a_value), original_value, bonus);
			}
		}
		return GetActorValueModifier(original_value + bonus, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_3(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_1(a_this, a_modifier, a_value);
		float bonus = 0.0;
		if (Plugin::InGame() && a_this) {
			if (a_this->formID == 0x14 && a_value == ActorValue::kHealth) {
				float value = (a_this->AsActorValueOwner()->GetBaseActorValue(ActorValue::kHealth)) - original_value;
				bonus = AttributeManager::AlterGetBaseAv(a_this, a_value, value);
				//log::info("GetAV3 true, modifier {}, value: {}, original_value: {}, bonus: {}", static_cast<uint32_t>(a_modifier), static_cast<uint32_t>(a_value), original_value, bonus);
			}
		}
		return GetActorValueModifier(original_value + bonus, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_4(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_1(a_this, a_modifier, a_value);
		float bonus = 0.0;
		if (Plugin::InGame() && a_this) {
			if (a_this->formID == 0x14 && a_value == ActorValue::kHealth) {
				float value = (a_this->AsActorValueOwner()->GetBaseActorValue(ActorValue::kHealth)) - original_value;
				bonus = AttributeManager::AlterGetBaseAv(a_this, a_value, value);
				//log::info("GetAV4 true, modifier {}, value: {}, original_value: {}, bonus: {}", a_modifier, a_value, original_value, bonus);
			}
		}
		return GetActorValueModifier(original_value + bonus, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_5(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_5(a_this, a_modifier, a_value);
		//log::info("GetAV5 true");
		return GetActorValueModifier(original_value, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_6(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_6(a_this, a_modifier, a_value);
		//log::info("GetAV6 true");
		return GetActorValueModifier(original_value, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_7(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_7(a_this, a_modifier, a_value);
		//log::info("GetAV7 true");
		return GetActorValueModifier(original_value, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_8(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_8(a_this, a_modifier, a_value);
		//log::info("GetAV8 true");
		return GetActorValueModifier(original_value, a_this, a_modifier, a_value);
	}

	float Hook_Actor::GetActorValueModifier_9(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		float original_value = _GetActorValueModifier_9(a_this, a_modifier, a_value);
		//log::info("GetAV9 true");
		return GetActorValueModifier(original_value, a_this, a_modifier, a_value);
	}

	NiPoint3 Hook_Actor::GetBoundMax(Actor* a_this) {
		auto bound = _GetBoundMax(a_this);
		if (a_this) {
			float scale = get_giantess_scale(a_this);
			if (scale > 1e-4) {
				bound = bound * scale;
			}
		}
		return bound;
	}
	NiPoint3 Hook_Actor::GetBoundMin(Actor* a_this) {
		auto bound = _GetBoundMin(a_this);
		if (a_this) {
			float scale = get_giantess_scale(a_this);
			if (scale > 1e-4) {
				bound = bound * scale;
			}
		}
		return bound;
	}
}
