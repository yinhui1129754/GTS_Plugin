#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "managers/InputManager.hpp"
#include "managers/Rumble.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>


using namespace Gts;
using namespace RE;
using namespace REL;
using namespace SKSE;

namespace {
	const double LAUNCH_COOLDOWN = 0.8f;
	const double DAMAGE_COOLDOWN = 2.0f;
	const double HANDDAMAGE_COOLDOWN = 0.6f;
	const double THIGHDAMAGE_COOLDOWN = 1.2f;
	const double HEALTHGATE_COOLDOWN = 60.0f;
	const double SCARE_COOLDOWN = 6.0f;
	const double BUTTCRUSH_COOLDOWN = 30.0f;
	const double HUGS_COOLDOWN = 8.0f;

	float Calculate_Halflife(CameraTracking Bone) {
		if (Bone == CameraTracking::Thigh_Crush) { // Thigh Crushing
			return 0.15;
		} else if (Bone == CameraTracking::VoreHand_Right || Bone == CameraTracking::Hand_Left || Bone == CameraTracking::Hand_Right) { // Voring / using hands
			return 0.10;
		} else if (Bone == CameraTracking::ObjectA || Bone == CameraTracking::ObjectB) { // pretty much vore/ hands too
			return 0.10;
		} else if (Bone == CameraTracking::R_Foot || Bone == CameraTracking::L_Foot) { // Feet
			return 0.08;
		} else if (Bone == CameraTracking::Butt || Bone == CameraTracking::Mid_Butt_Legs) { // Butt
			return 0.08;
		} else if (Bone == CameraTracking::Breasts_02) { // Breasts
			return 0.10;
		} else if (Bone == CameraTracking::Knees) { // Knees
			return 0.10;
		} else if (Bone == CameraTracking::Finger_Right || Bone == CameraTracking::Finger_Left) {
			return 0.08;
		} else {
			return 0.05;
		}
	}

	float get_endless_height(Actor* giant) {
		float endless = 0.0;
		if (Runtime::HasPerk(giant, "ColossalGrowth")) {
			endless = 99999999.0;
		}
		return endless;
	}
}

namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	std::string SizeManager::DebugName() {
		return "SizeManager";
	}

	void SizeManager::Update() {
		auto profiler = Profilers::Profile("SizeManager: Update");
		for (auto actor: find_actors()) {
			// 2023 + 2024: TODO: move away from polling
			float Endless = 0.0;
			if (actor->formID == 0x14) {
				Endless = get_endless_height(actor);
			}

			float GameScale = game_get_scale_overrides(actor);

			float NaturalScale = 1.0; //get_neutral_scale(actor);
			// ^ If we won't alter it - .dll will reset the scale to max scale
			float Gigantism = Ench_Aspect_GetPower(actor);

			float QuestStage = Runtime::GetStage("MainQuest");
			auto Persistent = Persistent::GetSingleton().GetData(actor);
			
			float GetLimit = std::clamp(NaturalScale + ((Runtime::GetFloat("sizeLimit") - 1.0f) * NaturalScale), NaturalScale, 99999999.0f); // Default size limit
			
			float Persistent_Size = 1.0;
			float SelectedFormula = Runtime::GetInt("SelectedSizeFormula");

			float FollowerLimit = Runtime::GetFloat("FollowersSizeLimit"); // 0 by default
			float NPCLimit = Runtime::GetFloat("NPCSizeLimit"); // 0 by default

			float buttcrush = GetButtCrushSize(actor);

			if (Persistent) {
				float size_overrides = Potion_GetSizeMultiplier(actor);
				// ^ Takes ButtCrush growth and Potion amplifiers into account
				Persistent_Size = ((1.0 + Persistent->bonus_max_size) * size_overrides);
			}

			if (actor->formID == 0x14 && SelectedFormula >= 1.0) { // Apply Player Mass-Based max size
				float low_limit = get_endless_height(actor);
				if (low_limit < 2) {
					low_limit = Runtime::GetFloat("sizeLimit");
				}
				GetLimit = std::clamp(NaturalScale + (Runtime::GetFloat("GtsMassBasedSize") * NaturalScale), NaturalScale, low_limit);
			} else if (QuestStage > 100 && FollowerLimit > 0.0 && FollowerLimit != 1.0 && actor->formID != 0x14 && IsTeammate(actor)) { // Apply Follower Max Size
				GetLimit = std::clamp(NaturalScale + ((Runtime::GetFloat("FollowersSizeLimit") - 1.0f) * NaturalScale), NaturalScale * FollowerLimit, 99999999.0f); // Apply only if Quest is done.
			} else if (QuestStage > 100 && NPCLimit > 0.0 && NPCLimit != 1.0 && actor->formID != 0x14 && !IsTeammate(actor)) { // Apply Other NPC's max size
				GetLimit = std::clamp(NaturalScale + ((Runtime::GetFloat("NPCSizeLimit") - 1.0f) * NaturalScale), NaturalScale * NPCLimit, 99999999.0f);       // Apply only if Quest is done.
			}

			float TotalLimit = (buttcrush + ((GetLimit * Persistent_Size) * (1.0 + Gigantism))) / GameScale;

			if (get_max_scale(actor) < TotalLimit + Endless || get_max_scale(actor) > TotalLimit + Endless) {
				set_max_scale(actor, TotalLimit);
			}
		}
	}

	void SizeManager::SetEnchantmentBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).enchantmentBonus = amt;
	}

	float SizeManager::GetEnchantmentBonus(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float EB = std::clamp(this->GetData(actor).enchantmentBonus, 0.0f, 1000.0f);
		return EB;
	}

	void SizeManager::ModEnchantmentBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).enchantmentBonus += amt;
	}

	//=================Size Hunger

	void SizeManager::SetSizeHungerBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).SizeHungerBonus = amt;
	}

	float SizeManager::GetSizeHungerBonus(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float SHB = std::clamp(this->GetData(actor).SizeHungerBonus, 0.0f, 1000.0f);
		return SHB;
	}

	void SizeManager::ModSizeHungerBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).SizeHungerBonus += amt;
	}

	//==================Growth Spurt

	void SizeManager::SetGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurtSize = amt;
	}

	float SizeManager::GetGrowthSpurt(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float GS = clamp (0.0, 999999.0, this->GetData(actor).GrowthSpurtSize);
		return GS;
	}

	void SizeManager::ModGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurtSize += amt;
	}

	//================Size-Related Damage
	void SizeManager::SetSizeAttribute(Actor* actor, float amt, SizeAttribute attribute) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (Persistent) {
			switch (attribute) {
				case SizeAttribute::Normal:
					Persistent->NormalDamage = amt;
				break;
				case SizeAttribute::Sprint:
					Persistent->SprintDamage = amt;
				break;
				case SizeAttribute::JumpFall: 
					Persistent->FallDamage = amt;
				break;
				case SizeAttribute::HighHeel:
					Persistent->HHDamage = amt;
				break;
			}
		}
	}

	float SizeManager::GetSizeAttribute(Actor* actor, SizeAttribute attribute) {
		if (!actor) {
			return 1.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (Persistent) {
			float Normal = clamp (1.0, 999999.0, Persistent->NormalDamage);
			float Sprint = clamp (1.0, 999999.0, Persistent->SprintDamage);
			float Fall = clamp (1.0, 999999.0, Persistent->FallDamage);
			float HH = clamp (1.0, 999999.0, Persistent->HHDamage);
			switch (attribute) {
				case SizeAttribute::Normal: 
					return Normal;
				break;
				case SizeAttribute::Sprint:
					return Sprint;
				break;
				case SizeAttribute::JumpFall:
					return Fall;
				break;
				case SizeAttribute::HighHeel:
					return GetHighHeelsBonusDamage(actor, false);
				break;
				}
			}
		return 1.0;
	}

	//===============Size-Related Attribute End


	//===============Size-Vulnerability

	void SizeManager::SetSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->SizeVulnerability = amt;
	}

	float SizeManager::GetSizeVulnerability(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 0.0;
		}
		return clamp(0.0, 3.0, Persistent->SizeVulnerability);
	}

	void SizeManager::ModSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (Persistent->SizeVulnerability < 3) {
			Persistent->SizeVulnerability += amt;
		}
	}
	//===============Size-Vulnerability

	//===============Hit Growth

	float SizeManager::GetHitGrowth(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 0.0;
		}
		
		return Persistent->AllowHitGrowth;
	}

	void SizeManager::SetHitGrowth(Actor* actor, float allow) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->AllowHitGrowth = allow;
	}

	//===============Size-Vulnerability

	//===============Camera Stuff
	void SizeManager::SetTrackedBone(Actor* actor, bool enable, CameraTracking Bone) {
		if (!enable) {
			Bone = CameraTracking::None;
		}
		SetCameraHalflife(actor, Bone);
		SetCameraOverride(actor, enable);
		this->GetData(actor).TrackedBone = Bone;
	}

	CameraTracking SizeManager::GetTrackedBone(Actor* actor) {
		return this->GetData(actor).TrackedBone;
	}


	//==============Half life stuff
	void SizeManager::SetCameraHalflife(Actor* actor, CameraTracking Bone) {
		this->GetData(actor).Camera_HalfLife = Calculate_Halflife(Bone);
	}

	float SizeManager::GetCameraHalflife(Actor* actor) {
		return this->GetData(actor).Camera_HalfLife;
	}
	//

	//===============Balance Mode
	float SizeManager::BalancedMode()
	{
		if (Runtime::GetBool("BalanceMode")) {
			return 2.0;
		} else {
			return 1.0;
		}
	}

	SizeManagerData& SizeManager::GetData(Actor* actor) {
		this->sizeData.try_emplace(actor);
		return this->sizeData.at(actor);
	}

	void SizeManager::Reset() {
		auto caster = PlayerCharacter::GetSingleton();
		if (caster) {
			SetEnchantmentBonus(caster, 0.0);
			SetGrowthSpurt(caster, 0.0);
		}
		
		TaskManager::CancelAllTasks(); // just in case, to avoid CTD
	}

}