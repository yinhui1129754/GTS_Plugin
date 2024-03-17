#pragma once
// Module that handles the Camera
#include "events.hpp"
#include "spring.hpp"
#include "timer.hpp"

#include "managers/cameras/state.hpp"
#include "managers/cameras/trans.hpp"
#include "managers/cameras/tp/alt.hpp"
#include "managers/cameras/tp/normal.hpp"
#include "managers/cameras/tp/foot.hpp"
#include "managers/cameras/tp/footL.hpp"
#include "managers/cameras/tp/footR.hpp"

#include "managers/cameras/fp/normal.hpp"
#include "managers/cameras/fp/combat.hpp"
#include "managers/cameras/fp/loot.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	enum class CameraMode {

	};

	class CameraManager : public EventListener {
		public:
			[[nodiscard]] static CameraManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void DataReady() override;
			virtual void Start() override;

			virtual void CameraUpdate() override;

			CameraState* GetCameraState();

			void AdjustUpDown(float amt);
			void ResetUpDown();

			void AdjustLeftRight(float amt);
			void ResetLeftRight();

		private:
			CameraState* GetCameraStateTP();
			CameraState* GetCameraStateFP();

			CameraState scaledVanillaState;  // Like vanilla only scaled

			Normal normalState;
			Alt altState;
			Foot footState;
			FootR footRState;
			FootL footLState;

			FirstPerson fpState;
			FirstPersonCombat fpCombatState;
			FirstPersonLoot fpLootState;

			NiPoint3 manualEdit;

			Timer initimer = Timer(3.00);

			Spring smoothScale = Spring(0.30, 0.50);
			Spring3 smoothOffset = Spring3(NiPoint3(0.30, 0.30, 0.30), 0.50);
			float CameraDelay = 0.0;

			CameraState* currentState = nullptr;
			std::unique_ptr<TransState> transitionState = std::unique_ptr<TransState>(nullptr);
	};
}