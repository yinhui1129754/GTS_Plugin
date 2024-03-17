#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	NiPoint3 GetContainerSpawnLocation(Actor* giant, Actor* tiny);
	void TransferInventory(Actor* from, Actor* to, const float scale, bool keepOwnership, bool removeQuestItems, DamageSource Cause, bool reset);
	void TransferInventory_Normal(Actor* giant, Actor* tiny, bool removeQuestItems);
	void TransferInventoryToDropbox(Actor* giant, Actor* actor, const float scale, bool removeQuestItems, DamageSource Cause, bool Resurrected);
	void MoveItemsTowardsDropbox(Actor* actor, TESObjectREFR* dropbox, bool removeQuestItems);
}