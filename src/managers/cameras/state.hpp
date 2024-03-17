#pragma once

using namespace RE;

namespace Gts {
	class CameraState {
		public:
			virtual void EnterState();
			virtual void ExitState();

			virtual float GetScale();
			virtual float GetScaleOverride(bool IsCrawling);

			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal, bool IsCrawling);
			virtual NiPoint3 GetOffsetProne(const NiPoint3& cameraPosLocal);

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal, bool IsCrawling);
			virtual NiPoint3 GetCombatOffsetProne(const NiPoint3& cameraPosLocal);

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool IsCrawling);
			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal);

			virtual bool PermitManualEdit();
			virtual bool PermitTransition();
			virtual bool PermitCameraTransforms();
	};
}
