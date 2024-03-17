#pragma once
// Module that handles AttributeAdjustment
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	struct SizeManagerData {
		float enchantmentBonus = 0.0;
		float SizeHungerBonus = 0.0;
		float HitGrowth = 0.0;
		float GrowthTimer = 0.0;
		float GrowthSpurtSize = 0.0;

		float NormalDamage = 1.0; // 0
		float SprintDamage = 1.0; // 1
		float FallDamage = 1.0; // 2
		float HHDamage = 1.0; // 3
		float SizeVulnerability = 0.0;

		CameraTracking TrackedBone = CameraTracking::None;
		float Camera_HalfLife = 0.05;
	};

	struct LaunchData {
		double lastLaunchTime = -1.0e8; 
	};

	struct DamageData {
		double lastDamageTime = -1.0e8;
		double lastHandDamageTime = -1.0e8;
		double lastHealthGateTime = -1.0e8;
		double lastThighDamageTime = -1.0e8;
		double lastButtCrushTime = -1.0e8;
		double lastScareTime = -1.0e8;
		double lastHugTime = -1.0e8;
	};

	class SizeManager : public EventListener {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			void Reset();

			SizeManagerData& GetData(Actor* actor);

			void SetEnchantmentBonus(Actor* actor, float amt);
			float GetEnchantmentBonus(Actor* actor);
			void ModEnchantmentBonus(Actor* actor, float amt);

			void SetSizeHungerBonus(Actor* actor, float amt);
			float GetSizeHungerBonus(Actor* actor);
			void ModSizeHungerBonus(Actor* actor, float amt);

			void SetGrowthSpurt(Actor* actor, float amt);
			float GetGrowthSpurt(Actor* actor);
			void ModGrowthSpurt(Actor* actor, float amt);

			void SetSizeAttribute(Actor* actor, float amt, float attribute);
			float GetSizeAttribute(Actor* actor, float attribute);
			void ModSizeAttribute(Actor* actor, float amt, float attribute);

			void SetSizeVulnerability(Actor* actor, float amt);
			float GetSizeVulnerability(Actor* actor);
			void ModSizeVulnerability(Actor* actor, float amt);

			float GetHitGrowth(Actor* actor);
			void SetHitGrowth(Actor* actor, float allow);

			void SetTrackedBone(Actor* actor, bool enable, CameraTracking Bone);
			CameraTracking GetTrackedBone(Actor* actor);

			void SetCameraHalflife(Actor* actor, CameraTracking Bone);
			float GetCameraHalflife(Actor* actor);
		

			float BalancedMode();

			LaunchData& GetLaunchData(Actor* actor);
			DamageData& GetDamageData(Actor* actor);

			static bool IsLaunching(Actor* actor);
			static bool IsDamaging(Actor* actor);
			static bool IsHandDamaging(Actor* actor);
			static bool IsThighDamaging(Actor* actor);
			static bool IsBeingScared(Actor* actor);
			static bool IsButtCrushInCooldown(Actor* actor);
			static bool IsHugsOnCooldown(Actor* actor);
			static bool IsHealthGateInCooldown(Actor* actor);

			bool GetPreciseDamage();

		private: 
			std::map<Actor*, SizeManagerData> sizeData;
			std::map<Actor*, LaunchData> launchData;
			std::map<Actor*, DamageData> DamageData;
	};
}
