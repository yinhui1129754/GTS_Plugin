#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	void HookCameraStates();

	class Hook_CameraState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_ThirdPersonState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_HorseState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_DragonState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_FirstPersonState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_FreeState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_TransitionState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_BleedoutState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_VATState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_FurnitureState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_IronSightState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_VanityState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_TweenState
	{
		public:
			static void Hook();
		private:

			static void Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}
