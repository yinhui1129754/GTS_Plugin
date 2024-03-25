#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    enum class ChestType {
		BossChest,
		MiniChest,
		MiscChest,
	};

    TESContainer* GetChestRef(FormID form, ChestType type);

    void DistributeChestItems();

    void AddItemToChests(FormID Chest);

    std::vector<TESForm*> FindAllChests();
   
	std::vector<TESBoundObject*> CalculateItemProbability(ChestType type);
}