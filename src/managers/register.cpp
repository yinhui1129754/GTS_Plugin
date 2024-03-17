
#include "managers/animation/AnimationManager.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/ai/headtracking.hpp"
#include "managers/ai/ai_Manager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/OverkillManager.hpp"
#include "managers/RandomGrowth.hpp"
#include "managers/Attributes.hpp"
#include "managers/GtsManager.hpp"
#include "managers/hitmanager.hpp"
#include "managers/footik/collider.hpp"
#include "managers/explosion.hpp"
#include "managers/register.hpp"
#include "managers/reloader.hpp"
#include "managers/highheel.hpp"
#include "managers/footstep.hpp"
#include "managers/contact.hpp"
#include "managers/camera.hpp"
#include "managers/tremor.hpp"
#include "managers/rumble.hpp"
#include "managers/vore.hpp"
#include "utils/DynamicScale.hpp"
#include "magic/magic.hpp"
#include "events.hpp"

namespace Gts {
	void RegisterManagers() {
		EventDispatcher::AddListener(&GameModeManager::GetSingleton()); // Manages Game Modes
		EventDispatcher::AddListener(&GtsManager::GetSingleton()); // Manages smooth size increase and animation & movement speed
		EventDispatcher::AddListener(&SizeManager::GetSingleton()); // Manages Max Scale of everyone
		EventDispatcher::AddListener(&HighHeelManager::GetSingleton()); // Applies high heels
		EventDispatcher::AddListener(&CameraManager::GetSingleton()); // Edits the camera
		EventDispatcher::AddListener(&ReloadManager::GetSingleton()); // Handles Skyrim Events
		EventDispatcher::AddListener(&CollisionDamage::GetSingleton()); // Handles precise size-related damage

		EventDispatcher::AddListener(&MagicManager::GetSingleton()); // Manages spells and size changes in general
		EventDispatcher::AddListener(&Vore::GetSingleton()); // Manages vore
		EventDispatcher::AddListener(&CrushManager::GetSingleton()); // Manages crushing
		EventDispatcher::AddListener(&OverkillManager::GetSingleton()); // Manages crushing
		EventDispatcher::AddListener(&ShrinkToNothingManager::GetSingleton()); // Shrink to nothing manager

		EventDispatcher::AddListener(&FootStepManager::GetSingleton()); // Manages footstep sounds
		EventDispatcher::AddListener(&TremorManager::GetSingleton()); // Manages tremors on footstop
		EventDispatcher::AddListener(&ExplosionManager::GetSingleton()); // Manages clouds/exposions on footstep
		EventDispatcher::AddListener(&GRumble::GetSingleton()); // Manages rumbling of contoller/camera for multiple frames

		EventDispatcher::AddListener(&AttributeManager::GetSingleton()); // Adjusts most attributes
		EventDispatcher::AddListener(&RandomGrowth::GetSingleton()); // Manages random growth perk
		//EventDispatcher::AddListener(&QuestManager::GetSingleton()); // Quest is currently empty and not needed
		EventDispatcher::AddListener(&HitManager::GetSingleton()); // Hit Manager for handleing papyrus hit events

		EventDispatcher::AddListener(&AnimationManager::GetSingleton()); // Manages Animation Events

		EventDispatcher::AddListener(&Grab::GetSingleton()); // Manages grabbing
		EventDispatcher::AddListener(&ThighSandwichController::GetSingleton()); // Manages Thigh Sandwiching

		EventDispatcher::AddListener(&AiManager::GetSingleton()); // Rough AI controller for GTS-actions
		EventDispatcher::AddListener(&Headtracking::GetSingleton()); // Headtracking fixes
		//EventDispatcher::AddListener(&ColliderManager::GetSingleton()); // Manage FootIK


		EventDispatcher::AddListener(&ContactManager::GetSingleton()); // Manages collisions
		EventDispatcher::AddListener(&InputManager::GetSingleton()); // Manages keyboard and mouse input

		EventDispatcher::AddListener(&DynamicScale::GetSingleton()); // Handles room heights
		log::info("Managers Registered");
	}
}
