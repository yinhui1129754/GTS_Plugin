#include "colliders/ragdoll.hpp"
#include "colliders/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	RagdollData::RagdollData(hkaRagdollInstance* ragdoll) {
		if (!ragdoll) {
			return;
		}

		for (auto rb: ragdoll->rigidBodies) {
			AddRB(rb);
		}
	}
}
