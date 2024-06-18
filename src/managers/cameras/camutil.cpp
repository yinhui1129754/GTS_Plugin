#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "scale/modscale.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "node.hpp"

using namespace RE;

namespace {
	enum class CameraDataMode {
		State,
		Transform,
	};

	const CameraDataMode currentMode = CameraDataMode::State;

	void CAMERA_ASM_TEST(float value) { // Crashes / doesn't work. 
		//FactorCameraOffset = 49866 ; 50799

		// sE: 14084B430 - 14084b448 = 0x18
		// ASM Attempt, doesn't work properly.

		constexpr REL::Offset FloatA_GetOffset_SE(0x84b430); // 14084b430 - 1404f54b5: 0x18
		constexpr REL::Offset FloatA_GetOffset_AE(50799); // 14084b430 - 14050e5f5: 0x95

		// 0x14084b430 - 0x14084b440 : 0x10  (value 1)
		// 0x14084b430 - 0x14084b454 : 0x1c  (value 2)
		// 0x14084b430 - 0x14084b459 : 0x29  (value 3)
		// 0x14084b430 - 0x14084f9f1 : 0x45C1  (in_RDX[2] (1))
		// 0x14084b430 - 0x14084f977 : 0x4547  (in_RDX[2] (2))
		// 0x14084b430 - 0x14084f9fb : 0x45CB  (in_RDX[1])
		// 0x1404f5420 - 0x14084f97e : 0x35A55E
		//REL::Relocation<std::uintptr_t> FloatA_SE_Hook(FloatA_GetOffset_SE, 0x18);
		//REL::Relocation<std::uintptr_t> FloatA_AE_Hook(FloatA_GetOffset_AE, 0x18);

		//REL::safe_write(ShakeCamera_GetOffset_SE_Hook.address(), targets);
		if (REL::Module::IsSE()) {
			log::info("SE Patched!");
			log::info("Value: {}", value);
			const float& pass = value;

			REL::safe_write(FloatA_GetOffset_SE.address() + 0x10, &pass, 8);
			REL::safe_write(FloatA_GetOffset_SE.address() + 0x1C, &pass, 8);
			REL::safe_write(FloatA_GetOffset_SE.address() + 0x29, &pass, 8);
			// ^ Do not work
			REL::safe_write(FloatA_GetOffset_SE.address() + 0x45C1, &value, 8);
			//REL::safe_write(FloatA_GetOffset_SE.address() + 0x45CB, &value, sizeof(float));
			//REL::safe_write(FloatA_GetOffset_SE.address() + 0x35A55E, &value, sizeof(float));
			// ^ Crash the game
			
		} else if (REL::Module::IsAE()) {
			log::info("AE Patched!");
			//REL::safe_write(FloatA_AE_Hook.address(), &targets, sizeof(float));
		}
	}

}

namespace Gts {

	float HighHeelOffset() {
		Actor* player = PlayerCharacter::GetSingleton();
		float hh = 0.0;
		if (player) {
			hh = HighHeelManager::GetBaseHHOffset(player).z;
			hh *= HighHeelManager::GetHHMultiplier(player);
			if (IsFootGrinding(player) || IsTrampling(player) || IsStomping(player) || IsVoring(player)) {
				hh = 0.0;
			}
		}
		return hh;
	}

	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}

	float GetINIFloat(std::string_view name) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			return setting->data.f;
		}
		return -1.0;
	}

	void EnsureINIFloat(std::string_view name, float value) {
		auto currentValue = GetINIFloat(name);
		if (fabs(currentValue - value) > 1e-3) {
			SetINIFloat(name, value);
		}
	}

	void UpdateThirdPerson() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = GetCameraActor();
		if (camera && player) {
			camera->UpdateThirdPerson(player->AsActorState()->IsWeaponDrawn());
		}
	}

	void ResetIniSettings() {
		EnsureINIFloat("fOverShoulderPosX:Camera", 30.0);
		EnsureINIFloat("fOverShoulderPosY:Camera", 30.0);
		EnsureINIFloat("fOverShoulderPosZ:Camera", -10.0);
		EnsureINIFloat("fOverShoulderCombatPosX:Camera", 0.0);
		EnsureINIFloat("fOverShoulderCombatPosY:Camera", 0.0);
		EnsureINIFloat("fOverShoulderCombatPosZ:Camera", 20.0);
		EnsureINIFloat("fVanityModeMaxDist:Camera", 600.0);
		EnsureINIFloat("fVanityModeMinDist:Camera", 155.0);
		EnsureINIFloat("fMouseWheelZoomSpeed:Camera", 0.8000000119);
		EnsureINIFloat("fMouseWheelZoomIncrement:Camera", 0.075000003);
		UpdateThirdPerson();
	}

	NiCamera* GetNiCamera() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
		for (auto child: cameraRoot->GetChildren()) {
			NiAVObject* node = child.get();
			if (node) {
				NiCamera* casted = netimmerse_cast<NiCamera*>(node);
				if (casted) {
					niCamera = casted;
					break;
				}
			}
		}
		return niCamera;
	}
	void UpdateWorld2ScreetMat(NiCamera* niCamera) {
		auto camNi = niCamera ? niCamera : GetNiCamera();
		typedef void (*UpdateWorldToScreenMtx)(RE::NiCamera*);
		static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(REL::RelocationID(69271, 70641).address());
		toScreenFunc(camNi);
	}

	Actor* GetCameraActor() {
		auto camera = PlayerCamera::GetSingleton();
		return camera->cameraTarget.get().get();
	}


	void UpdateSceneManager(NiPoint3 camLoc) {
		auto sceneManager = UI3DSceneManager::GetSingleton();
		if (sceneManager) {
			// Cache
			sceneManager->cachedCameraPos = camLoc;

			/*#ifdef ENABLED_SHADOW
			   // Shadow Map
			   auto shadowNode = sceneManager->shadowSceneNode;
			   if (shadowNode) {
			        shadowNode->GetRuntimeData().cameraPos = camLoc;
			   }
			 #endif*/

			// Camera
			auto niCamera = sceneManager->camera;
			if (niCamera) {
				niCamera->world.translate = camLoc;
				UpdateWorld2ScreetMat(niCamera.get());
			}
		}
	}

	void UpdateRenderManager(NiPoint3 camLoc) {
		auto renderManager = UIRenderManager::GetSingleton();
		if (renderManager) {
			/*#ifdef ENABLED_SHADOW
			   // Shadow Map
			   auto shadowNode = renderManager->shadowSceneNode;
			   if (shadowNode) {
			        shadowNode->GetRuntimeData().cameraPos = camLoc;
			   }
			 #endif*/

			// Camera
			auto niCamera = renderManager->camera;
			if (niCamera) {
				niCamera->world.translate = camLoc;
				UpdateWorld2ScreetMat(niCamera.get());
			}
		}
	}

	void UpdateNiCamera(NiPoint3 camLoc) {
		auto niCamera = GetNiCamera();
		if (niCamera) {
			niCamera->world.translate = camLoc;
			UpdateWorld2ScreetMat(niCamera);
			update_node(niCamera);
		}

		/*#ifdef ENABLED_SHADOW
		   auto shadowNode = GetShadowMap();
		   if (shadowNode) {
		        shadowNode->GetRuntimeData().cameraPos = camLoc;
		   }
		 #endif*/
	}

	NiTransform GetCameraWorldTransform() {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto cameraRoot = camera->cameraRoot;
			if (cameraRoot) {
				return cameraRoot->world;
			}
		}
		return NiTransform();
	}

	void UpdatePlayerCamera(NiPoint3 camLoc) {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto cameraRoot = camera->cameraRoot;
			if (cameraRoot) {
				auto cameraState = reinterpret_cast<ThirdPersonState*>(camera->currentState.get());
				cameraRoot->local.translate = camLoc;
				cameraRoot->world.translate = camLoc;
				update_node(cameraRoot.get());
			}
		}
	}

	NiPoint3 GetCameraPosition() {
		NiPoint3 cameraLocation;
		switch (currentMode) {
			case CameraDataMode::State: {
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto currentState = camera->currentState;
					if (currentState) {
						currentState->GetTranslation(cameraLocation);
					}
				}
			}
			case CameraDataMode::Transform: {
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto cameraRoot = camera->cameraRoot;
					if (cameraRoot) {
						cameraLocation = cameraRoot->world.translate;
					}
				}
			}
		}
		return cameraLocation;
	}

	NiMatrix3 GetCameraRotation() {
		NiMatrix3 cameraRot;
		switch (currentMode) {
			case CameraDataMode::State: {
				//log::info("Camera State: State");
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto currentState = camera->currentState;
					if (currentState) {
						NiQuaternion cameraQuat;
						currentState->GetRotation(cameraQuat);
						cameraRot = QuatToMatrix(cameraQuat);
					}
				}
			}
			case CameraDataMode::Transform: {
				//log::info("Camera State: Transform");
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto cameraRoot = camera->cameraRoot;
					if (cameraRoot) {
						cameraRot = cameraRoot->world.rotate;
					}
				}
			}
		}
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto currentState = camera->currentState;
			if (currentState) {
				NiQuaternion cameraQuat;
				currentState->GetRotation(cameraQuat);
				cameraRot = QuatToMatrix(cameraQuat);
			}
		}
			
		return cameraRot;
	}

	// Get's camera position relative to the player
	NiPoint3 GetCameraPosLocal() {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			NiPointer<TESObjectREFR> Target = camera->cameraTarget.get();

			auto currentState = camera->currentState;
			if (currentState) {
				auto player = GetCameraActor();
				if (player) {
					auto model = player->Get3D(false);
					if (model) {
						NiPoint3 cameraLocation = GetCameraPosition();
						auto playerTrans = model->world;
						playerTrans.scale = model->parent ? model->parent->world.scale : 1.0; // Only do translation/rotation
						auto playerTransInve = playerTrans.Invert();
						// Get Scaled Camera Location
						return playerTransInve*cameraLocation;
					}
				}
			}
		}
		return NiPoint3();
	}

	NiMatrix3 QuatToMatrix(const NiQuaternion& q){
		float sqw = q.w*q.w;
		float sqx = q.x*q.x;
		float sqy = q.y*q.y;
		float sqz = q.z*q.z;

		// invs (inverse square length) is only required if quaternion is not already normalised
		float invs = 1 / (sqx + sqy + sqz + sqw);
		float m00 = ( sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
		float m11 = (-sqx + sqy - sqz + sqw)*invs;
		float m22 = (-sqx - sqy + sqz + sqw)*invs;

		float tmp1 = q.x*q.y;
		float tmp2 = q.z*q.w;
		float m10 = 2.0 * (tmp1 + tmp2)*invs;
		float m01 = 2.0 * (tmp1 - tmp2)*invs;

		tmp1 = q.x*q.z;
		tmp2 = q.y*q.w;
		float m20 = 2.0 * (tmp1 - tmp2)*invs;
		float m02 = 2.0 * (tmp1 + tmp2)*invs;
		tmp1 = q.y*q.z;
		tmp2 = q.x*q.w;
		float m21 = 2.0 * (tmp1 + tmp2)*invs;
		float m12 = 2.0 * (tmp1 - tmp2)*invs;

		return NiMatrix3(
			NiPoint3(m00, m01, m02),
			NiPoint3(m10, m11, m12),
			NiPoint3(m20, m21, m22)
			);
	}

	NiPoint3 FirstPersonPoint() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kFirstPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}
	NiPoint3 ThirdPersonPoint() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kThirdPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}

	float ZoomFactor() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kThirdPerson].get();
		if (camState) {
			ThirdPersonState* tpState = skyrim_cast<ThirdPersonState*>(camState);
			if (tpState) {
				return tpState->currentZoomOffset;
			}
		}
		return 0.0;
	}
	float MaxZoom() {
		return GetINIFloat("fVanityModeMaxDist:Camera");
	}

	NiPoint3 CompuleLookAt(float zoomScale) {
		NiPoint3 cameraTrans = GetCameraPosition();

		NiMatrix3 cameraRotMat = GetCameraRotation();

		float zoomOffset = ZoomFactor() * MaxZoom() * zoomScale;
		NiPoint3 zoomOffsetVec = NiPoint3(0.0, zoomOffset, 0.0);
		return cameraRotMat * zoomOffsetVec + cameraTrans;
	}

	void UpdateCamera(float scale, NiPoint3 cameraLocalOffset, NiPoint3 playerLocalOffset) {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot;
		auto player = GetCameraActor();
		auto currentState = camera->currentState;

		float value = Runtime::GetFloatOr("cameraAlternateX", 1.0);

		//CAMERA_ASM_TEST(value);

		if (cameraRoot) {
			if (currentState) {
				auto cameraWorldTranform = GetCameraWorldTransform();
				NiPoint3 cameraLocation;
				currentState->GetTranslation(cameraLocation);
				if (player) {
					if (scale > 1e-4) {
						auto model = player->Get3D(false);
						if (model) {
							auto playerTrans = model->world;
							playerTrans.scale = model->parent ? model->parent->world.scale : 1.0;  // Only do translation/rotation
							auto playerTransInve = playerTrans.Invert();

							// Make the transform matrix for our changes
							NiTransform adjustments = NiTransform();
							adjustments.scale = scale;
							// Adjust by scale reports 1.0 / naturalscale (Which includes RaceMenu and GetScale)


							adjustments.translate = playerLocalOffset;

							// Get Scaled Camera Location
							auto targetLocationWorld = playerTrans*(adjustments*(playerTransInve*cameraLocation));

							// Get shifted Camera Location
							cameraWorldTranform.translate = targetLocationWorld; // Update with latest position
							NiTransform adjustments2 = NiTransform();
							adjustments2.translate = cameraLocalOffset * scale;
							auto worldShifted =  cameraWorldTranform * adjustments2 * NiPoint3();

							// Convert to local space
							auto parent = cameraRoot->parent;
							NiTransform transform = parent->world.Invert();
							auto localShifted = transform * worldShifted;
							auto targetLocationLocalShifted = localShifted;

							UpdatePlayerCamera(targetLocationLocalShifted);
							UpdateNiCamera(targetLocationLocalShifted);

							//UpdateSceneManager(targetLocationLocalShifted);
							//UpdateRenderManager(targetLocationLocalShifted);
						}
					}
				}
			}
		}
	}

}