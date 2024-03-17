#pragma once
#include "managers/cameras/state.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class TransState : public CameraState {
		public:
			virtual float GetScale() override;
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) override;
			virtual NiPoint3 GetOffsetProne(const NiPoint3& cameraPosLocal) override;

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) override;
			virtual NiPoint3 GetCombatOffsetProne(const NiPoint3& cameraPosLocal) override;

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) override;
			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) override;

			virtual bool PermitManualEdit() override;
			virtual bool PermitTransition() override;
			virtual bool PermitCameraTransforms() override;

			TransState(CameraState* stateA, CameraState* stateB);

			bool IsDone();
		private:
			CameraState* stateA;
			CameraState* stateB;
			Spring smoothIn = Spring(0.0, 0.4);
	};
}
