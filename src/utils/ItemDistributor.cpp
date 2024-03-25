#include "utils/ItemDistributor.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "colliders/RE.hpp"
#include "rays/raycast.hpp"
#include "UI/DebugAPI.hpp"
#include "utils/av.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"


#include <vector>
#include <string>


using namespace RE;
using namespace Gts;

namespace {
	// Boss Chests
	const FormID BossChest_Giant = 		0x774BF; // TreastGiantChestBoss
	const FormID BossChest_Bandit = 	0x2064F; // TreasBanditChestBoss
	const FormID BossChest_Draugr =     0x20671; // TreasDraugrChestBoss

	// Mini Chests
	const FormID NormalChest_Normal =     0x3AC21;	// TreasBanditChest
	const FormID NormalChest_Giant =      0x774C6;    // TreasGiantChest

	// Barrels and misc
	const FormID Barrel_1 =     		0x845; 		// Barrel 1
	const FormID Barrel_2 =             0xFE078806; // Barrel 2
	const FormID Long_Crate_1 =         0xFE05D819; // Long Crate 1
	const FormID Long_Crate_2 =         0xFE05D81A; // Long Crate 2

    const std::vector<FormID> BossChests = {
        BossChest_Giant,
        BossChest_Bandit,
        BossChest_Draugr,
    };

    const std::vector<FormID> NormalChests = {
        NormalChest_Normal,
        NormalChest_Giant,
    };

    const std::vector<FormID> MiscChests = {
        Barrel_1,
        Barrel_2,
        Long_Crate_1,
        Long_Crate_2,
    };
}

namespace Gts {
    TESContainer* GetChestRef(TESForm* form, ChestType type) {
        switch (type) {
            case ChestType::BossChest: {
                for (auto chest: BossChests) {
                    if (chest == form->formID) {
                        return form->As<RE::TESContainer>();
                    }
                }
                break;
            }
            case ChestType::NormalChest: {
                for (auto chest: NormalChests) {
                    if (chest == form->formID) {
                        return form->As<RE::TESContainer>();
                    }
                }
                break;
            }
            case ChestType::MiscChest: {
                for (auto chest: MiscChests) {
                    if (chest == form->formID) {
                        return form->As<RE::TESContainer>();
                    }
                }
                break;
            }
        }
        return nullptr;
    }

    void DistributeChestItems() {
        float QuestStage = Runtime::GetStage("MainQuest");
        if (QuestStage < 20) {
            return;
        }
        for (auto Chest: FindAllChests()) {
            if (Chest) {
                AddItemToChests(Chest);
            }
        }
    }

    void AddItemToChests(TESForm* Chest) {
        TESContainer* container_Boss = GetChestRef(Chest, ChestType::BossChest); 
        TESContainer* container_Normal = GetChestRef(Chest, ChestType::NormalChest); 
        TESContainer* container_Misc = GetChestRef(Chest, ChestType::MiscChest); 
        if (container_Boss) {
            log::info("Boss container found!");
            for (auto item: CalculateItemProbability(ChestType::BossChest)) {
                if (item) {
                    log::info("Adding Boss items");
                    container_Boss->AddObjectToContainer(item, 1, nullptr);
                }
            }
        }
        else if (container_Normal) {
            log::info("Normal chest found!");
            for (auto item: CalculateItemProbability(ChestType::NormalChest)) {
                if (item) {
                    log::info("Adding Normal items");
                    container_Normal->AddObjectToContainer(item, 1, nullptr);
                }
            }
        }
        else if (container_Misc) {
            log::info("Misc chest found!");
            for (auto item: CalculateItemProbability(ChestType::MiscChest)) {
                if (item) {
                    log::info("Adding items");
                    container_Misc->AddObjectToContainer(item, 1, nullptr);
                }
            }
        }
    }

    std::vector<TESForm*> FindAllChests() {
        RE::TESDataHandler* const DataHandler = RE::TESDataHandler::GetSingleton();

        std::vector<TESForm*> Forms = {}; 
        auto containers = DataHandler->GetFormArray(RE::FormType::Container);
        for (auto container: containers) {
            Forms.push_back(container);
        }

        if (Forms.size() < 1) {
            log::info("Forms are empty");
            return {};
        }

        return Forms;
    }

    std::vector<TESBoundObject*> CalculateItemProbability(ChestType type) {
        float HighLootChance = Runtime::GetStage("MainQuest");
        float Level = 1.0 + GetGtsSkillLevel() * 0.01;
        float ChanceToAdd = 100;
        int rng = rand() % 98;
        log::info("Calculating item probability");

        switch (type) {
            case (ChestType::BossChest): { // Always add stuff
                ChanceToAdd = 100;
                break;
            }
            case (ChestType::NormalChest): { // Occasionally add stuff
                ChanceToAdd = 15 * Level;
                break;
            }
            case (ChestType::MiscChest): { // Very small chance to add stuff
                ChanceToAdd = 0.15 * Level;
                break;
            }
        }

        if (rng <= ChanceToAdd) { // Add only if RNG returns true
            log::info("RNG Rolled true, adding items");
            return SelectItemsFromPool(type, Level * 100.0);
        }
        return {};
    }

    std::vector<TESLevItem*> SelectItemsFromPool(ChestType type, float Level) {
        TESLevItem* ResistSize_Weak = Runtime::GetLeveledItem("Potion_ResistSize_Weak");
        TESLevItem* ResistSize = Runtime::GetLeveledItem("Potion_ResistSize");
        TESLevItem* Growth = Runtime::GetLeveledItem("Potion_Growth");

        TESLevItem* SizeLimit_Weak = Runtime::GetLeveledItem("Potion_SizeLimit_Weak");
        TESLevItem* SizeLimit_Normal = Runtime::GetLeveledItem("Potion_SizeLimit_Normal");
        TESLevItem* SizeLimit_Strong = Runtime::GetLeveledItem("Potion_SizeLimit_Strong");
        TESLevItem* SizeLimit_Extreme = Runtime::GetLeveledItem("Potion_SizeLimit_Extreme");

        TESLevItem* SizeHunger_Weak = Runtime::GetLeveledItem("Potion_SizeHunger_Weak");
        TESLevItem* SizeHunger_Normal = Runtime::GetLeveledItem("Potion_SizeHunger_Normal");
        TESLevItem* SizeHunger_Strong = Runtime::GetLeveledItem("Potion_SizeHunger_Strong");
        TESLevItem* SizeHunger_Extreme = Runtime::GetLeveledItem("Potion_SizeHunger_Extreme");

        TESLevItem* Size_Amplify = Runtime::GetLeveledItem("Potion_Size_Amplify");
        TESLevItem* Size_Drain = Runtime::GetLeveledItem("Poison_Size_Drain");
        TESLevItem* Size_Shrink = Runtime::GetLeveledItem("Poison_Size_Shrink");

        TESLevItem* Amulet = Runtime::GetLeveledItem("AmuletOfGiants");

        std::vector<TESLevItem*> ChosenItems = {};

        const std::vector<TESLevItem*> WeakPotions = {
            SizeHunger_Weak,
            SizeLimit_Weak,
            ResistSize_Weak,
            Growth,
        };
        const std::vector<TESLevItem*> NormalPotions = {
            SizeHunger_Normal,
            SizeLimit_Normal,
            Size_Drain,
            Amulet,
        };
        const std::vector<TESLevItem*> StrongPotions = {
            SizeHunger_Strong,
            SizeLimit_Strong,
            Size_Amplify,
            ResistSize,
        };
        const std::vector<TESLevItem*> ExtremePotions = {
            SizeHunger_Extreme,
            SizeLimit_Extreme,
            Size_Shrink,
            Size_Drain,
        };
        
        float weakBoundary = 100;
        float normalBoundary = 1 + Level * 10;
        float strongBoundary = 1 + Level * Level * 1;
        float extremeBoundary = 1 + Level * Level * Level * 0.1;

        float totalPercentage = weakBoundary + normalBoundary + strongBoundary + extremeBoundary;
        weakBoundary = weakBoundary / totalPercentage * 100;
        normalBoundary = normalBoundary / totalPercentage * 100;
        strongBoundary = strongBoundary / totalPercentage * 100;
        extremeBoundary = extremeBoundary / totalPercentage * 100;

        int roll = rand() % 100; // Select item rarity
        int MaxItems = 1 + (rand() % 1 + 1); // Limit amount of items that can be spawned
        MaxItems *= (Level * 0.01);
        
        int SelectItem; // Select random item from array

        if (type == ChestType::NormalChest) {
            roll *= 0.60;
        } else if (type == ChestType::MiscChest) {
            roll *= 0.10;
        }
        log::info("rolled max items: {}", MaxItems);

        for (int i = 0; i < MaxItems; ++i) { // Run the func multiple times 
            if (roll > weakBoundary + normalBoundary + strongBoundary) {
                SelectItem = rand() % (ExtremePotions.size() + 1);
                ChosenItems.push_back(ExtremePotions[SelectItem]);
                log::info("Extreme Random: {}", SelectItem);
            } else if (roll > weakBoundary + normalBoundary) {
                SelectItem = rand() % (StrongPotions.size() + 1);
                ChosenItems.push_back(StrongPotions[SelectItem]);
                log::info("Strong Random: {}", SelectItem);
            } else if (roll > weakBoundary) {
                SelectItem = rand() % (NormalPotions.size() + 1);
                ChosenItems.push_back(NormalPotions[SelectItem]);
                log::info("Normal Random: {}", SelectItem);
            } else {
                SelectItem = rand() % (WeakPotions.size() + 1);
                ChosenItems.push_back(WeakPotions[SelectItem]);
                log::info("Weak Random: {}", SelectItem);
            }
        }

        return ChosenItems;
    }
}