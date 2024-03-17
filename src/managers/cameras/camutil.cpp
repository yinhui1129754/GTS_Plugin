#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "node.hpp"

using namespace RE;

namespace {
	enum class CameraDataMode {
		State,
		Transform,
	};

	const CameraDataMode currentMode = CameraDataMode::State;

}

namespace Gts {
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