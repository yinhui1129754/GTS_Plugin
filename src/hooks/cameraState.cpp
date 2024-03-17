#include "hooks/cameraState.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "events.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void HookCameraStates() {
		Hook_CameraState::Hook();
		Hook_ThirdPersonState::Hook();
		Hook_DragonState::Hook();
		Hook_HorseState::Hook();
		Hook_FirstPersonState::Hook();
		Hook_FreeState::Hook();
		Hook_TransitionState::Hook();
		Hook_BleedoutState::Hook();
		Hook_VATState::Hook();
		Hook_FurnitureState::Hook();
		Hook_IronSightState::Hook();
		Hook_VanityState::Hook();
		Hook_TweenState::Hook();
	};

	// Base
	void Hook_CameraState::Hook() {
		log::info("Hooking TESCameraState");
		REL::Relocation<std::uintptr_t> Vtbl{ TESCameraState::VTABLE[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_CameraState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Third
	void Hook_ThirdPersonState::Hook() {
		log::info("Hooking ThirdPersonState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_ThirdPersonState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_ThirdPersonState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Dragon
	void Hook_DragonState::Hook() {
		log::info("Hooking DragonState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_DragonCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_DragonState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Horse
	void Hook_HorseState::Hook() {
		log::info("Hooking HorseState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_HorseCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_HorseState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// First
	void Hook_FirstPersonState::Hook() {
		log::info("Hooking FirstPersonState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_FirstPersonState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_FirstPersonState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// FreeCamera
	void Hook_FreeState::Hook() {
		log::info("Hooking FreeCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_FreeCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_FreeState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Transistion
	void Hook_TransitionState::Hook() {
		log::info("Hooking TransitionCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_PlayerCameraTransitionState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_TransitionState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Bleedout
	void Hook_BleedoutState::Hook() {
		log::info("Hooking BleedoutCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_BleedoutCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_BleedoutState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Vats
	void Hook_VATState::Hook() {
		log::info("Hooking VATCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_VATSCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_VATState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Furniture
	void Hook_FurnitureState::Hook() {
		log::info("Hooking Furniture");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_FurnitureCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_FurnitureState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// IronSights
	void Hook_IronSightState::Hook() {
		log::info("Hooking IronSightState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_IronSightsState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_IronSightState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Vanity
	void Hook_VanityState::Hook() {
		log::info("Hooking VanityState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_AutoVanityState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_VanityState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}

	// Tween
	void Hook_TweenState::Hook() {
		log::info("Hooking TweenCameraState");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_TweenMenuCameraState[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
	}

	void Hook_TweenState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		_Update(a_this, a_nextState);
		EventDispatcher::DoCameraUpdate();
	}



}
