#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Actor
	{
		public:
			static void Hook(Trampoline& trampoline);
		private:
			static void HandleHealthDamage(Actor* a_this, Actor* a_attacker, float a_damage);
			static inline REL::Relocation<decltype(HandleHealthDamage)> _HandleHealthDamage;

			static void AddPerk(Actor* a_this, BGSPerk* a_perk, std::uint32_t a_rank);
			static inline REL::Relocation<decltype(AddPerk)> _AddPerk;

			static void RemovePerk(Actor* a_this, BGSPerk* a_perk);
			static inline REL::Relocation<decltype(RemovePerk)> _RemovePerk;

			static float  GetActorValue(ActorValueOwner* a_this, ActorValue a_akValue);
			static inline REL::Relocation<decltype(GetActorValue)> _GetActorValue;

			static float  GetBaseActorValue(ActorValueOwner* a_this, ActorValue a_akValue);
			static inline REL::Relocation<decltype(GetBaseActorValue)> _GetBaseActorValue;

			static void SetBaseActorValue(ActorValueOwner* a_this, ActorValue a_akValue, float a_value);
			static inline REL::Relocation<decltype(SetBaseActorValue)> _SetBaseActorValue;

			static float GetPermanentActorValue(ActorValueOwner* a_this, ActorValue a_akValue);
			static inline REL::Relocation<decltype(GetPermanentActorValue)> _GetPermanentActorValue;

			static void Move(Actor* a_this, float a_arg2, const NiPoint3& a_position);
			static inline REL::Relocation<decltype(Move)> _Move;

			static float GetActorValueModifier_1(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_1)> _GetActorValueModifier_1;

			static float GetActorValueModifier_2(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_2)> _GetActorValueModifier_2;

			static float GetActorValueModifier_3(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_3)> _GetActorValueModifier_3;

			static float GetActorValueModifier_4(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_4)> _GetActorValueModifier_4;

			static float GetActorValueModifier_5(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_5)> _GetActorValueModifier_5;

			static float GetActorValueModifier_6(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_6)> _GetActorValueModifier_6;

			static float GetActorValueModifier_7(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_7)> _GetActorValueModifier_7;

			static float GetActorValueModifier_8(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_8)> _GetActorValueModifier_8;

			static float GetActorValueModifier_9(Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
			static inline REL::Relocation<decltype(GetActorValueModifier_9)> _GetActorValueModifier_9;

			static NiPoint3 GetBoundMin(Actor* a_this);
			static inline REL::Relocation<decltype(GetBoundMin)> _GetBoundMin;

			static NiPoint3 GetBoundMax(Actor* a_this);
			static inline REL::Relocation<decltype(GetBoundMax)> _GetBoundMax;
	};
}
