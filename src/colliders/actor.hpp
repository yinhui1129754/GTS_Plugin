#pragma once
// Takes a actor and extracts its collision objects
#include "colliders/common.hpp"
#include "colliders/charcontroller.hpp"
#include "colliders/ragdoll.hpp"


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class ActorCollisionData : public ColliderData {
		public:
			ActorCollisionData(Actor* actor);
		protected:
			virtual std::vector<ColliderData*> GetChildren() override;
		private:
			CharContData charCont;
			RagdollData ragdoll;
	};
}
