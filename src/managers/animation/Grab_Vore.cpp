#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/Grab_Vore.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;

namespace {
    void GTSGrab_Eat_Start(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		ManageCamera(&data.giant, true, CameraTracking::Grab_Left);
		if (otherActor) {
			VoreData.AddTiny(otherActor);
		}
		StartLHandRumble("GrabVoreL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Eat_OpenMouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		if (otherActor) {
			SetBeingEaten(otherActor, true);
		}
		AdjustFacialExpression(giant, 0, 1.0, "phenome"); // Start opening mouth
		AdjustFacialExpression(giant, 1, 0.5, "phenome"); // Open it wider

		AdjustFacialExpression(giant, 0, 0.80, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.80, "modifier"); // blink R

		AdjustFacialExpression(&data.giant, 3, 0.8, "phenome"); // Smile a bit (Mouth)
		StopLHandRumble("GrabVoreL", data.giant);
	}

	void GTSGrab_Eat_Eat(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		if (otherActor) {
			for (auto& tiny: VoreData.GetVories()) {
				if (!AllowDevourment()) {
					VoreData.Swallow();
					if (IsCrawling(&data.giant)) {
						otherActor->SetAlpha(0.0); // Hide Actor
					}
				} else {
					CallDevourment(&data.giant, otherActor);
				}
			}
		}
	}

	void GTSGrab_Eat_CloseMouth(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "phenome"); // Close mouth
		AdjustFacialExpression(giant, 1, 0.0, "phenome"); // Close it

		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R

		AdjustFacialExpression(&data.giant, 3, 0.0, "phenome"); // Smile a bit (Mouth)
	}

	void GTSGrab_Eat_Swallow(AnimationEventData& data) {
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);
		if (otherActor) {
			SetBeingEaten(otherActor, false);
			auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
			for (auto& tiny: VoreData.GetVories()) {
				VoreData.KillAll();
			}
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
			Runtime::PlaySoundAtNode("VoreSwallow", &data.giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
			AnimationManager::StartAnim("TinyDied", giant);
			//BlockFirstPerson(giant, false);
			ManageCamera(&data.giant, false, CameraTracking::Grab_Left);
			SetBeingHeld(otherActor, false);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}
	}
}

namespace Gts {
    void Animation_GrabVore::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSGrab_Eat_Start", "Grabbing", GTSGrab_Eat_Start);
		AnimationManager::RegisterEvent("GTSGrab_Eat_OpenMouth", "Grabbing", GTSGrab_Eat_OpenMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Eat", "Grabbing", GTSGrab_Eat_Eat);
		AnimationManager::RegisterEvent("GTSGrab_Eat_CloseMouth", "Grabbing", GTSGrab_Eat_CloseMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Swallow", "Grabbing", GTSGrab_Eat_Swallow);
    }
}