// Animation: Compatibility
// Notes: Made avaliable for other generic anim mods
//  - Stages
//    - "GTScrush_caster",          //[0] The gainer.
//    - "GTScrush_victim",          //[1] The one to crush
// Notes: Modern Combat Overhaul compatibility
// - Stages
//   - "MCO_SecondDodge",           // enables GTS sounds and footstep effects
//   - "SoundPlay.MCO_DodgeSound",

#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/Compat.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void TriggerKillZone(Actor* giant) {
		if (!giant) {
			return;
		}
		float BASE_CHECK_DISTANCE = 90.0;
		float SCALE_RATIO = 3.0;
		if (HasSMT(giant)) {
			SCALE_RATIO = 0.8;
		}
		float giantScale = get_visual_scale(giant);
		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				if (otherActor->IsInKillMove()) {
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale * 3) {
							PrintDeathSource(giant, otherActor, DamageSource::Booty);
							CrushManager::Crush(giant, otherActor);
						}
					}
				}
			}
		}
	}

	void GTScrush_caster(AnimationEventData& data) { 
		// Compatibility with Thick Thighs Take Lives mod, this compatibility probably needs a revision.
		// Mainly just need to call damage similar to how we do it with DoDamageAtPoint() function
		// 21.01.2024
		//data.stage = 0;
		TriggerKillZone(&data.giant);
	}

	void GTScrush_victim(AnimationEventData& data) { // Compatibility with Thick Thighs Take Lives mod
		//data.stage = 0;
		if (data.giant.formID != 0x14) {
			TriggerKillZone(PlayerCharacter::GetSingleton());
		}
	}

	void MCO_SecondDodge(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		DoDamageEffect(&data.giant, Damage_Stomp, Radius_Stomp, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoDamageEffect(&data.giant, Damage_Stomp, Radius_Stomp, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Right);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Left);
	}
	void MCO_DodgeSound(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		DoDamageEffect(&data.giant, Damage_Stomp, Radius_Stomp, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoDamageEffect(&data.giant, Damage_Stomp, Radius_Stomp, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Right);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Left);
	}

	
}

namespace Gts
{
	void AnimationCompat::RegisterEvents() {
		AnimationManager::RegisterEvent("GTScrush_caster", "Compat1", GTScrush_caster);
		AnimationManager::RegisterEvent("GTScrush_victim", "Compat2", GTScrush_victim);
		AnimationManager::RegisterEvent("MCO_SecondDodge", "MCOCompat1", MCO_SecondDodge);
		AnimationManager::RegisterEvent("SoundPlay.MCO_DodgeSound", "MCOCompat2", MCO_DodgeSound);
	}

	void AnimationCompat::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Tiny_ExitAnims", "Compat3", "GTSBEH_Tiny_Abort");
	}
}
