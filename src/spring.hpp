#pragma once
// Critically Damped Springs
//
#include "events.hpp"

using namespace SKSE;

namespace Gts {
	class SpringBase {
		public:
			virtual void Update(float delta) = 0;
		protected:
			void UpdateValues(float& value, const float& target, float & velocity, const float& halflife, const float& dt);
	};

	class Spring : public SpringBase {
		public:
			float value = 0.0;
			float target = 0.0;
			float velocity = 0.0;
			float halflife = 1.0;

			void Update(float delta) override;

			Spring();
			Spring(float initial, float halflife);

			~Spring();
	};

	class Spring3 : public SpringBase {
		public:
			NiPoint3 value = NiPoint3(0.0, 0.0, 0.0);
			NiPoint3 target = NiPoint3(0.0, 0.0, 0.0);
			NiPoint3 velocity = NiPoint3(0.0, 0.0, 0.0);
			float halflife = 1.0;

			void Update(float delta) override;

			Spring3();
			Spring3(NiPoint3 initial, float halflife);

			~Spring3();
	};

	class SpringManager : public EventListener {
		public:
			static SpringManager& GetSingleton();

			static void AddSpring(SpringBase* spring);
			static void RemoveSpring(SpringBase* spring);

			virtual std::string DebugName() override;
			virtual void Update() override;

			std::unordered_set<SpringBase*> springs;
	};
}
