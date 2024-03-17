#include "managers/ShrinkToNothingManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/hitmanager.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "utils/looting.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;
namespace {
	float GetPushPower(float sizeRatio) {
		// https://www.desmos.com/calculator/wh0vwgljfl
		SoftPotential push {
			.k = 1.42,
			.n = 0.78,
			.s = 0.50,
			.a = 0.0,
		};
		float power = soft_power(sizeRatio, push);
		return power;
	}
}

namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	std::string HitManager::DebugName() {
		return "HitManager";
	}

	void HitManager::HitEvent(const TESHitEvent* a_event) {
		if (!a_event) {
			return;
		}
		auto attacker_PTR = a_event->cause;
		auto atacker_ref = attacker_PTR.get();
		auto attacker = skyrim_cast<Actor*>(atacker_ref);
		if (!attacker) {
			return;
		}
		auto receiver_PTR = a_event->target;
		auto receiver_ref = receiver_PTR.get();
		auto receiver = skyrim_cast<Actor*>(receiver_ref);
		if (!receiver) {
			return;
		}

		auto& sizemanager = SizeManager::GetSingleton();
		auto& Persist = Persistent::GetSingleton();

		auto HitIdForm = a_event->source;
		auto HitId = TESForm::LookupByID(HitIdForm);

		std::string hitName = HitId->GetName();

		if (hitName == "Stagger" || hitName == "SizeEffect" || hitName == "SprintingSizeEffect" || hitName == "GtsTastyFoe") {
			return;
		}
		auto ProjectileIDForm = a_event->projectile;
		auto ProjectileID = TESForm::LookupByID(ProjectileIDForm);
		auto player = PlayerCharacter::GetSingleton();

		bool wasPowerAttack = a_event->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_event->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_event->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_event->flags.all(TESHitEvent::Flag::kHitBlocked);
		static Timer timer = Timer(0.25);

		float attackerscale = get_visual_scale(attacker);
		float receiverscale = get_visual_scale(receiver) * GetSizeFromBoundingBox(receiver);

		float size_difference = attackerscale/receiverscale;

		if (HasSMT(player)) {
			size_difference += 3.0;
		}

		// Apply it
		float pushpower = GetPushPower(size_difference);
		if (attacker->formID == 0x14 && size_difference >= 4.0) {
			FormType formType = HitId->GetFormType();
			if (formType != FormType::Weapon) {
				return;
			}
			if (wasPowerAttack || hitName.find("Bow") != std::string::npos) {
				size_difference *= 2.0;
				pushpower *= 2.0;
			}
			if (hitName.find("Bow") == std::string::npos) {
				shake_camera(attacker, size_difference * 0.20, 0.35);
			}
			PushForward(attacker, receiver, pushpower * 25);
			//log::info("Size difference is met, pushing actor away");
		}
	}

	void HitManager::Update() {
		// unused
		return;
	}
}
