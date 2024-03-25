#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    enum class ChestType {
		BossChest,
		NormalChest,
		MiscChest,
	};

    TESContainer* GetChestRef(TESForm* form, ChestType type);

    void DistributeChestItems();

    void AddItemToChests(TESForm* Chest);

    std::vector<TESForm*> FindAllChests();
   
	std::vector<TESLevItem*> CalculateItemProbability(ChestType type);
    std::vector<TESBoundObject*> SelectItemsFromPool(ChestType type, float Level);
}