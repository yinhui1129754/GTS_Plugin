#include "managers/cameras/camutil.hpp"
#include "managers/cameras/tp/alt.hpp"
#include "managers/GtsSizeManager.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Alt::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}

	// fVanityModeMaxDist:Camera Changes The Offset Value We Need So we need to take this value into account;
	void Alt::SetZOff(float Offset) {
		// The 0.15 was found through testing different fVanityModeMaxDist values
		Alt::ZOffset = Offset - (0.15 * Gts::MaxZoom());
	}

	BoneTarget Alt::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		int MCM_Mode = Runtime::GetInt("AltCameraTarget");
		CameraTracking_MCM Camera_MCM = static_cast<CameraTracking_MCM>(MCM_Mode);
		CameraTracking Camera_Anim = sizemanager.GetTrackedBone(player);

		float offset = -45;

		if (Camera_Anim != CameraTracking::None) { // must take priority
			switch (Camera_Anim) {
				case CameraTracking::None: {
					return BoneTarget();
				}
				case CameraTracking::Butt: {
					return BoneTarget {
							.boneNames = {
							"NPC L Butt",
							"NPC R Butt",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking::Knees: {
					return BoneTarget {
							.boneNames = {
							"NPC L Calf [LClf]",
							"NPC R Calf [RClf]",
						},
							.zoomScale = 1.25,
					};
				}
				case CameraTracking::Breasts_02: {
					return BoneTarget {
							.boneNames = {
							"L Breast02",
							"R Breast02",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking::Thigh_Crush: {
					return BoneTarget {
							.boneNames = {
							"NPC R PreRearCalf",
							"NPC R Foot [Rft ]",
							"NPC L PreRearCalf",
							"NPC L Foot [Lft ]",
						},
							.zoomScale = 1.00,
					};
				}
				case CameraTracking::Thigh_Sandwich: {
					return BoneTarget {
							.boneNames = {
							"AnimObjectA",
						},
							.zoomScale = 1.00,
					};
				}
				case CameraTracking::Hand_Right: {
					return BoneTarget {
							.boneNames = {
							"NPC R Hand [RHnd]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking::Hand_Left: {
					return BoneTarget {
							.boneNames = {
							"NPC L Hand [LHnd]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking::Grab_Left: {
					return BoneTarget {
							.boneNames = {
							"NPC L Finger02 [LF02]",
						},
							.zoomScale = 0.60,
					};
				}
				case CameraTracking::L_Foot: {
					return BoneTarget {
							.boneNames = {
							"NPC L Foot [Lft ]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking::R_Foot: {
					return BoneTarget {
							.boneNames = {
							"NPC R Foot [Rft ]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking::Mid_Butt_Legs: {
					return BoneTarget {
							.boneNames = {
							"NPC L Butt",
							"NPC R Butt",
							"NPC L Foot [Lft ]",
							"NPC R Foot [Rft ]",
						},
							.zoomScale = 1.25,
					};
				}
				case CameraTracking::VoreHand_Right: {
					return BoneTarget {
							.boneNames = {
							"AnimObjectA",
						},
							.zoomScale = 1.25,
					};
				}
				case CameraTracking::Finger_Right: {
					return BoneTarget {
							.boneNames = {
							"NPC R Finger12 [RF12]",
						},
							.zoomScale = 0.50,
					};
				}
				case CameraTracking::Finger_Left: {
					return BoneTarget {
							.boneNames = {
							"NPC L Finger12 [LF12]",
						},
							.zoomScale = 0.50,
					};
				}
				case CameraTracking::ObjectA: {
					return BoneTarget {
							.boneNames = {
							"AnimObjectA",
						},
							.zoomScale = 1.0,
					};
				}
				case CameraTracking::ObjectB: {
					return BoneTarget {
							.boneNames = {
							"AnimObjectB",
						},
							.zoomScale = 1.0,
					};
				}
			}
		} else {
			switch (Camera_MCM) {
				case CameraTracking_MCM::None: {
					return BoneTarget();
				}
				case CameraTracking_MCM::Spine: {
					return BoneTarget {
							.boneNames = {
							"NPC Spine2 [Spn2]",
							"NPC Neck [Neck]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking_MCM::Clavicle: {
					return BoneTarget {
							.boneNames = {
							"NPC R Clavicle [RClv]",
							"NPC L Clavicle [LClv]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking_MCM::Breasts_01: {
					return BoneTarget {
							.boneNames = {
							"NPC L Breast",
							"NPC R Breast",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking_MCM::Breasts_02: {
					return BoneTarget {
							.boneNames = {
							"L Breast02",
							"R Breast02",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking_MCM::Breasts_03: {
					return BoneTarget {
							.boneNames = {
							"L Breast03",
							"R Breast03",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking_MCM::Neck: {
					return BoneTarget {
							.boneNames = {
							"NPC Neck [Neck]",
						},
							.zoomScale = 0.75,
					};
				}
				case CameraTracking_MCM::Butt: {
					return BoneTarget {
							.boneNames = {
							"NPC L Butt",
							"NPC R Butt",
						},
							.zoomScale = 0.75,
					};
				}
			}
		}
		return BoneTarget();
	}
}
