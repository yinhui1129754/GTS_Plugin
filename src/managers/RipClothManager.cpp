#include "managers/RipClothManager.hpp"

#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {

#define RANDOM_OFFSET (0.05 * ((rand() % 5) + 1.0))


	// List of keywords (Editor ID's) we want to ignore when stripping
	static const std::vector<string> KeywordBlackList = {
		"SOS_Genitals" //Fix Slot 52 Genitals while still keeping the ability to unequip slot 52 underwear
	};

	//List of slots we want to check
	static const std::vector<BGSBipedObjectForm::BipedObjectSlot> VallidSlots = {
		BGSBipedObjectForm::BipedObjectSlot::kHead,                 // 30
		// BGSBipedObjectForm::BipedObjectSlot::kHair,				// 31
		BGSBipedObjectForm::BipedObjectSlot::kBody,                 // 32
		BGSBipedObjectForm::BipedObjectSlot::kHands,                // 33
		BGSBipedObjectForm::BipedObjectSlot::kForearms,             // 34
		// BGSBipedObjectForm::BipedObjectSlot::kAmulet,			// 35
		// BGSBipedObjectForm::BipedObjectSlot::kRing,				// 36
		BGSBipedObjectForm::BipedObjectSlot::kFeet,                 // 37
		BGSBipedObjectForm::BipedObjectSlot::kCalves,               // 38
		// BGSBipedObjectForm::BipedObjectSlot::kShield,			// 39
		// BGSBipedObjectForm::BipedObjectSlot::kTail,				// 40
		// BGSBipedObjectForm::BipedObjectSlot::kLongHair,			// 41
		BGSBipedObjectForm::BipedObjectSlot::kCirclet,              // 42
		BGSBipedObjectForm::BipedObjectSlot::kEars,                 // 43
		BGSBipedObjectForm::BipedObjectSlot::kModMouth,             // 44
		BGSBipedObjectForm::BipedObjectSlot::kModNeck,              // 45
		BGSBipedObjectForm::BipedObjectSlot::kModChestPrimary,      // 46
		BGSBipedObjectForm::BipedObjectSlot::kModBack,              // 47
		BGSBipedObjectForm::BipedObjectSlot::kModMisc1,             // 48
		BGSBipedObjectForm::BipedObjectSlot::kModPelvisPrimary,     // 49
		// BGSBipedObjectForm::BipedObjectSlot::kDecapitateHead,	// 50
		// BGSBipedObjectForm::BipedObjectSlot::kDecapitate,		// 51
		BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary,   // 52
		// BGSBipedObjectForm::BipedObjectSlot::kModLegRight,		// 53
		BGSBipedObjectForm::BipedObjectSlot::kModLegLeft,           // 54
		// BGSBipedObjectForm::BipedObjectSlot::kModFaceJewelry,	// 55
		BGSBipedObjectForm::BipedObjectSlot::kModChestSecondary,    // 56
		BGSBipedObjectForm::BipedObjectSlot::kModShoulder,          // 57
		// BGSBipedObjectForm::BipedObjectSlot::kModArmLeft,		// 58
		BGSBipedObjectForm::BipedObjectSlot::kModArmRight,          // 59
		// BGSBipedObjectForm::BipedObjectSlot::kModMisc2,			// 60
		// BGSBipedObjectForm::BipedObjectSlot::kFX01,				// 61
	};

	ClothManager& ClothManager::GetSingleton() noexcept {
		static ClothManager instance;
		return instance;
	}

	std::string ClothManager::DebugName() {
		return "ClothManager";
	}

	//Get All Whitelisted Armors from an Actor
	std::vector<TESObjectARMO*> ParseArmors(RE::Actor *Act) {
		std::vector<TESObjectARMO *> ArmorList;
		for (auto Slot : VallidSlots) {

			auto Armor = Act->GetWornArmor(Slot);
			// If armor is null skip
			if (!Armor) {
				continue;
			}

			for (const auto &BKwd : KeywordBlackList) {
				if (Armor->HasKeywordString(BKwd)) {
					continue;    //If blacklisted keyword is found skip
				}
				ArmorList.push_back(Armor);                                             //Else Add it to the vector
			}
		}
		return ArmorList;
	}

	//Have We Shrinked Since The Last Call?
	int IsShrinking(float Scale) {
		static float LastScale;
		// If Current Scale is Equal or Larger, we either growed or stayed the same so no shriking happened
		bool Shrinking = !(Scale >= LastScale);
		LastScale = Scale;
		//log::info("Shrinking: {}", Shrinking);
		return Shrinking;
	}

	void DoRip(RE::Actor* Act, RE::TESObjectARMO* Slot) {

		if(Slot == nullptr) {
			return;
		}

		PlayMoanSound(Act, 1.0);
		auto manager = RE::ActorEquipManager::GetSingleton();
		// log::info("Rip");
		manager->UnequipObject(Act, Slot);
		Runtime::PlaySound("ClothTearSound", Act, 1.0, 1.0);
		GRumble::Once("ClothManager", Act, (32 * get_visual_scale(Act)), 0.05);
	}

	void ClothManager::CheckRip() {
		if (Runtime::GetFloat("AllowClothTearing") == 0.0) {
			return; // Abort doing anything if not set to 1

		}
		static Timer timer = Timer(1.2);

		if (!timer.ShouldRunFrame()) {
			return;
		}

		//log::info("CheckRip Run");
		auto PlayerPtr = PlayerCharacter::GetSingleton();
		if (!PlayerPtr) {
			return;
		}
		float CurrentScale = get_visual_scale(PlayerPtr);

		if (CurrentScale <= TearThreshold) {
			// log::info("Too Small {} should be {}", CurrentScale, TearThreshold);
			AddedThreshold = 0;
			return;  // Less than tear threshold? Dont Run and initialize / reset AddedThreshold incase of game load
		}

		//Player is Shrinking, Don't Run
		if (IsShrinking(CurrentScale)) {
			//log::info("Currently Shrinking {}", CurrentScale);
			//if shrinking recalculated the needed threshold so its always slightly above the current scale,
			//this prevents ripping after we stop shrinking
			AddedThreshold = CurrentScale - TearThreshold + RANDOM_OFFSET;
			return;
		}

		std::vector<TESObjectARMO *> Armors = ParseArmors(PlayerPtr);

		auto ArmorCount = Armors.size();
		if (ArmorCount == 0) {
			return; //No Vallid Armors Found

		}
		// Select Random Armor To Possibly Remove
		auto RandomSlot = rand() % ArmorCount;
		//log::info("Selected Index for armor: {}", RandomSlot);
		auto ArmorSlot = Armors[RandomSlot];

		// If True it means we just loaded in and are larger than TearThreshold but not too big
		// If we Arent too big
		// Set AddedThreshold To be just above Current Scale so ripping doesnt happen on game load
		if (AddedThreshold == -1.0 && !(CurrentScale > TooBig)) {
			AddedThreshold = CurrentScale - TearThreshold + RANDOM_OFFSET;
			// log::info("OnGameLoad Added Threshold: {}", AddedThreshold);
			return;
		}

		// Rip Imediatly if too big
		if (CurrentScale > TooBig) {
			// log::info("Too Big: {}", CurrentScale);
			AddedThreshold = 0;
			DoRip(PlayerPtr, ArmorSlot);
			return;
		}

		//if we meet scale conditions
		if (CurrentScale >= (TearThreshold + AddedThreshold)) {
			AddedThreshold += RANDOM_OFFSET;
			DoRip(PlayerPtr, ArmorSlot);
			return;
		}

		//log::info("No Condition Ran! Threshold: {} Added: {} Current {}", TearThreshold, AddedThreshold, CurrentScale);
	}
}
