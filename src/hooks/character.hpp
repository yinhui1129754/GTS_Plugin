#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Character
	{
		public:
			static void Hook();
		private:
			static void HandleHealthDamage(Character* a_this, Character* a_attacker, float a_damage);
			static inline REL::Relocation<decltype(HandleHealthDamage)> _HandleHealthDamage;

			static void AddPerk(Character* a_this, BGSPerk* a_perk, std::uint32_t a_rank);
			static inline REL::Relocation<decltype(AddPerk)> _AddPerk;

			static void RemovePerk(Character* a_this, BGSPerk* a_perk);
			static inline REL::Relocation<decltype(RemovePerk)> _RemovePerk;

			static float  GetActorValue(ActorValueOwner* a_this, ActorValue a_akValue);
			static inline REL::Relocation<decltype(GetActorValue)> _GetActorValue;

			static float GetPermanentActorValue(ActorValueOwner* a_this, ActorValue a_akValue);
			static inline REL::Relocation<decltype(GetPermanentActorValue)> _GetPermanentActorValue;

			static float GetBaseActorValue(ActorValueOwner* a_this, ActorValue a_akValue);
			static inline REL::Relocation<decltype(GetBaseActorValue)> _GetBaseActorValue;

			static void SetBaseActorValue(ActorValueOwner* a_this, ActorValue a_akValue, float a_value);
			static inline REL::Relocation<decltype(SetBaseActorValue)> _SetBaseActorValue;

			static void Move(Character* a_this, float a_arg2, const NiPoint3& a_position);
			static inline REL::Relocation<decltype(Move)> _Move;

			static void ProcessTracking(Character* a_this, float a_delta, NiAVObject* a_obj3D);
			static inline REL::Relocation<decltype(ProcessTracking)> _ProcessTracking;

			static void NPCAnimEvents(BSTEventSink<BSAnimationGraphEvent>* a_this, BSAnimationGraphEvent& a_event, BSTEventSource<BSAnimationGraphEvent>* a_src);
			static inline REL::Relocation<decltype(NPCAnimEvents)> _NPCAnimEvents;

			static NiPoint3 GetBoundMin(Character* a_this);
			static inline REL::Relocation<decltype(GetBoundMin)> _GetBoundMin;

			static NiPoint3 GetBoundMax(Character* a_this);
			static inline REL::Relocation<decltype(GetBoundMax)> _GetBoundMax;
	};
}
