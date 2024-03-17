#pragma once
// Takes a ragdoll and extracts its collision objects
#include "colliders/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	// Enable/disable collisions fails for current implementation
	//
	// A ragdoll goes from kBiped into kDeadBip depending on
	// if they are active/inactive
	class RagdollData : public ColliderData {
		public:
			RagdollData(hkaRagdollInstance* ragdoll);
	};
}
