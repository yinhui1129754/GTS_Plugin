#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class AttributeManager : public EventListener {
		public:
			[[nodiscard]] static AttributeManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			void OverrideSMTBonus(float Value);
			float GetAttributeBonus(Actor* actor, ActorValue av);

			static float AlterGetAv(Actor* actor, ActorValue av, float originalValue);
			static float AlterGetBaseAv(Actor* actor, ActorValue av, float originalValue);
			static float AlterSetBaseAv(Actor* actor, ActorValue av, float originalValue);
			static float AlterGetPermenantAv(Actor* actor, ActorValue av, float originalValue);
			static float AlterMovementSpeed(Actor* actor, const NiPoint3& direction);
			static float AlterGetAvMod(float orginal_value, Actor* a_this, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value);
		private:
			SoftPotential speed_adjustment_walk {
				.k = 0.265, // 0.125
				.n = 1.11, // 0.86
				.s = 2.0, // 1.12
				.o = 1.0,
				.a = 0.0,  //Default is 0
			};
	};
}
