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
	const FormID BossChest_Giant = 		0x774BF; // TreasGiantChestBoss
	const FormID BossChest_Bandit = 	0x2064F; // TreasBanditChestBoss
	const FormID BossChest_Draugr =     0x20671; // TreasDraugrChestBoss
    const FormID BossChest_Vampire =    0x20664; // TreasVampireChestBoss
    const FormID BossChest_Afflicted =  0x8EA5D; // TreasAfflictedChestBoss
    const FormID BossChest_ImperialL =  0x8B1F0; // TreasCWImperialChestBossLarge
    const FormID BossChest_SonsL =      0x8B1F1; // TreasCWSonsChestBossLarge
    const FormID BossChest_Dwarwen =    0x20652; // TreasDwarvenChestBoss
    const FormID BossChest_Falmer =     0x2065B; // TreasFalmerChestBoss
    const FormID BossChest_DWFalmer =   0xB1176; // TreasFalmerChestBossDwarven
    const FormID BossChest_Forsworn =   0x20658; // TreasForswornChestBoss
    const FormID BossChest_Hagraven =   0x20667; // TreasHagravenChestBoss
    const FormID BossChest_Orc      =   0x774C9; // TreasOrcChestBoss
    const FormID BossChest_Warlock  =   0x2065D; // TreasWarlockChestBoss
    const FormID BossChest_Werewolf =   0x20661; // TreasWerewolfChestBoss
    const FormID BossChest_DLC01_Elf =  0x2019DD6; // DLC01TreasSnowElfChestBoss
    const FormID BossChest_DLC01_SC =   0x20040A5; // DLC01SC_ChestBoss
    
	// Normal Chests
	const FormID NormalChest_Normal =     0x3AC21;	// TreasBanditChest
	const FormID NormalChest_Giant =      0x774C6;  // TreasGiantChest
    const FormID NormalChest_SonsS =      0x8B1E9;  // TreasCWSonsChestBossSmall
    const FormID NormalChest_ImperialS =  0x8B1E8; // TreasCWImperialChestBossSmall

	// Barrels and misc
	const FormID Barrel_1 =     		0x845; 		// Barrel 1
	const FormID Barrel_2 =             0xFE078806; // Barrel 2
	const FormID Long_Crate_1 =         0xFE05D819; // Long Crate 1
	const FormID Long_Crate_2 =         0xFE05D81A; // Long Crate 2

    const std::vector<FormID> BossChests = {
        BossChest_Giant,
        BossChest_Bandit,
        BossChest_Draugr,
        BossChest_Vampire,
        BossChest_Afflicted,
        BossChest_ImperialL,
        BossChest_SonsL,
        BossChest_Dwarwen,
        BossChest_Falmer,
        BossChest_DWFalmer,
        BossChest_Forsworn,
        BossChest_Hagraven,
        BossChest_Orc,
        BossChest_Warlock,
        BossChest_Werewolf,
        BossChest_DLC01_Elf,
        BossChest_DLC01_SC,
    };

    const std::vector<FormID> NormalChests = {
        NormalChest_Normal,
        NormalChest_Giant,
        NormalChest_SonsS,
        NormalChest_ImperialS,
    };

    const std::vector<FormID> MiscChests = {
        Barrel_1,
        Barrel_2,
        Long_Crate_1,
        Long_Crate_2,
    };
}

namespace Gts {

    TESContainer* FilterChests(TESForm* form, ChestType type) {
        switch (type) {
            case ChestType::BossChest: {
                for (auto chest: BossChests) {
                    if (chest == form->formID) {
                        log::info("BossChest Found");
                        return form->As<RE::TESContainer>();
                    }
                }
                break;
            }
            case ChestType::NormalChest: {
                for (auto chest: NormalChests) {
                    if (chest == form->formID) {
                        log::info("NormalChest Found");
                        return form->As<RE::TESContainer>();
                    }
                }
                break;
            }
            case ChestType::MiscChest: {
                for (auto chest: MiscChests) {
                    if (chest == form->formID) {
                        log::info("MiscChest Found");
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
            log::info("Quest stage is <20, returning");
            return;
        }
        for (auto Chest: FindAllChests()) {
            if (Chest) {
                AddItemToChests(Chest);
            }
        }
    }

    void AddItemToChests(TESForm* Chest) {
        TESContainer* container_Boss = FilterChests(Chest, ChestType::BossChest); 
        TESContainer* container_Normal = FilterChests(Chest, ChestType::NormalChest); 
        TESContainer* container_Misc = FilterChests(Chest, ChestType::MiscChest);

        if (container_Boss) {
            log::info("Boss container found!");
            for (auto item: CalculateItemProbability(ChestType::BossChest)) {
                if (item) {
                    container_Boss->AddObjectToContainer(item->As<RE::TESBoundObject>(), 1, nullptr);
                }
            }
        }
        else if (container_Normal) {
            log::info("Normal chest found!");
            for (auto item: CalculateItemProbability(ChestType::NormalChest)) {
                if (item) {
                    container_Normal->AddObjectToContainer(item->As<RE::TESBoundObject>(), 1, nullptr);
                }
            }
        }
        else if (container_Misc) {
            log::info("Misc chest found!");
            for (auto item: CalculateItemProbability(ChestType::MiscChest)) {
                if (item) {
                    container_Misc->AddObjectToContainer(item->As<RE::TESBoundObject>(), 1, nullptr);
                }
            }
        }
    }

    std::vector<TESForm*> FindAllChests() {
        RE::TESDataHandler* const DataHandler = RE::TESDataHandler::GetSingleton();

        std::vector<TESForm*> Forms = {}; 
        for (auto cont = DataHandler->GetFormArray(RE::FormType::Container).begin(); cont != DataHandler->GetFormArray(RE::FormType::Container).end(); ++cont) {
            Forms.push_back(*cont);
        }

        if (Forms.empty()) {
            log::info("Forms are empty");
            return {};
        }

        return Forms;
    }

    std::vector<TESLevItem*> CalculateItemProbability(ChestType type) {
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
                ChanceToAdd = 8 * Level;
                break;
            }
            case (ChestType::MiscChest): { // Very small chance to add stuff
                ChanceToAdd = 0.10 * Level;
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
        float strongBoundary = 1 + Level * Level * 0.5;
        float extremeBoundary = 1 + Level * Level * Level * 0.025;

        float totalPercentage = weakBoundary + normalBoundary + strongBoundary + extremeBoundary;
        weakBoundary = weakBoundary / totalPercentage * 100;
        normalBoundary = normalBoundary / totalPercentage * 100;
        strongBoundary = strongBoundary / totalPercentage * 100;
        extremeBoundary = extremeBoundary / totalPercentage * 100;

        
        int MaxItems = 1 + (rand() % 2); // Limit amount of items that can be spawned
        MaxItems *= (1.0 + Level * 0.01);
        
        int SelectItem; // Select random item from array

        
        log::info("rolled max items: {}", MaxItems);

        for (int i = 0; i < MaxItems; ++i) { // Run the func multiple times 

            int roll = rand() % 100; // Select item rarity

            if (type == ChestType::NormalChest) { // Worse rarity for normal chests
                roll *= 0.60;
            } else if (type == ChestType::MiscChest) { // Even worse rarity for trash-tier, barrels and such
                roll *= 0.10;
            }

            if (roll > weakBoundary + normalBoundary + strongBoundary) {
                SelectItem = rand() % (ExtremePotions.size());
                ChosenItems.push_back(ExtremePotions[SelectItem]);
                log::info("Extreme Random: {}", SelectItem);
            } else if (roll > weakBoundary + normalBoundary) {
                SelectItem = rand() % (StrongPotions.size());
                ChosenItems.push_back(StrongPotions[SelectItem]);
                log::info("Strong Random: {}", SelectItem);
            } else if (roll > weakBoundary) {
                SelectItem = rand() % (NormalPotions.size());
                ChosenItems.push_back(NormalPotions[SelectItem]);
                log::info("Normal Random: {}", SelectItem);
            } else {
                SelectItem = rand() % (WeakPotions.size());
                ChosenItems.push_back(WeakPotions[SelectItem]);
                log::info("Weak Random: {}", SelectItem);
            }
        }

        return ChosenItems;
    }
}