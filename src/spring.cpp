#include "spring.hpp"
#include "data/time.hpp"

namespace {
	// Spring code from https://theorangeduck.com/page/spring-roll-call
	float halflife_to_damping(float halflife, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (halflife + eps);
	}

	float damping_to_halflife(float damping, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (damping + eps);
	}
	float fast_negexp(float x)
	{
		return 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
	}
}

namespace Gts {

	void SpringBase::UpdateValues(float& value, const float& target, float & velocity, const float& halflife, const float& dt) {
		if (std::isinf(target)) {
			return;
		}
		if (fabs(target - value) < 1e-4 && velocity < 1e-4) {
			return;
		}
		float y = halflife_to_damping(halflife) / 2.0f;
		float j0 = value - target;
		float j1 = velocity + j0*y;
		float eydt = fast_negexp(y*dt);

		value = eydt*(j0 + j1*dt) + target;
		velocity = eydt*(velocity - j1*y*dt);
	}

	void Spring::Update(float dt) {
		UpdateValues(this->value, this->target, this->velocity, this->halflife, dt);
	}

	Spring::Spring() {
		SpringManager::AddSpring(this);
	}

	Spring::Spring(float initial, float halflife) : value(initial), target(initial), halflife(halflife) {
		SpringManager::AddSpring(this);
	}

	Spring::~Spring() {
		SpringManager::RemoveSpring(this);
	}



	void Spring3::Update(float dt) {
		UpdateValues(this->value.x, this->target.x, this->velocity.x, this->halflife, dt);
		UpdateValues(this->value.y, this->target.y, this->velocity.y, this->halflife, dt);
		UpdateValues(this->value.z, this->target.z, this->velocity.z, this->halflife, dt);
	}

	Spring3::Spring3() {
		SpringManager::AddSpring(this);
	}

	Spring3::Spring3(NiPoint3 initial, float halflife) : value(initial), target(initial), halflife(halflife) {
		SpringManager::AddSpring(this);
	}

	Spring3::~Spring3() {
		SpringManager::RemoveSpring(this);
	}


	SpringManager& SpringManager::GetSingleton() {
		static SpringManager instance;
		return instance;
	}

	void SpringManager::AddSpring(SpringBase* spring)  {
		SpringManager::GetSingleton().springs.insert(spring);
	}
	void SpringManager::RemoveSpring(SpringBase* spring) {
		SpringManager::GetSingleton().springs.erase(spring);
	}

	std::string SpringManager::DebugName()  {
		return "SpringManager";
	}

	void SpringManager::Update() {
		float dt = Time::WorldTimeDelta();
		for (auto spring: this->springs) {
			spring->Update(dt);
		}
		// log::info("Spring manager updated: {} spring", this->springs.size());
	}
}
