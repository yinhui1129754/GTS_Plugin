#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/animation/Grab_Sneak_Vore.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    void GTS_GrabSneak_Start(AnimationEventData& data) { // Register Tiny for Vore
		auto otherActor = Grab::GetHeldActor(&data.giant);

        ManageCamera(&data.giant, true, CameraTracking::Grab_Left);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);

		if (otherActor) {
			VoreData.AddTiny(otherActor);
		}
	}

    void GTS_GrabSneak_Eat(AnimationEventData& data) { 
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		for (auto& tiny: VoreData.GetVories()) {
            if (tiny) {
                tiny->NotifyAnimationGraph("JumpFall");
                Attacked(tiny, &data.giant);
            }
			VoreData.GrabAll(); // Switch to AnimObjectA attachment
		}
	}

	void GTS_GrabSneak_CamOff(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::ObjectA);
		ManageCamera(&data.giant, false, CameraTracking::Grab_Left);
	}

    void GTS_GrabSneak_KillAll(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		for (auto& tiny: VoreData.GetVories()) {
			if (tiny) {
				AllowToBeCrushed(tiny, true);
				EnableCollisions(tiny);
			}
		}
		VoreData.AllowToBeVored(true);
		VoreData.KillAll();
		VoreData.ReleaseAll();
    }
    // Rest is handled inside Vore_Sneak (some events are re-used)
}

namespace Gts {
    void Animation_GrabSneak_Vore::RegisterEvents() { 
		AnimationManager::RegisterEvent("GTS_GrabSneak_Start", "SneakVore", GTS_GrabSneak_Start);
        AnimationManager::RegisterEvent("GTS_GrabSneak_Eat", "SneakVore", GTS_GrabSneak_Eat);
		AnimationManager::RegisterEvent("GTS_GrabSneak_CamOff", "SneakVore", GTS_GrabSneak_CamOff);
        AnimationManager::RegisterEvent("GTS_GrabSneak_KillAll", "SneakVore", GTS_GrabSneak_KillAll);
    }
}