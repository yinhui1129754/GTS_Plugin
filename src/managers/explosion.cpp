#include "managers/GtsManager.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/impact.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "rays/raycast.hpp"


using namespace SKSE;
using namespace RE;
using namespace Gts;
using namespace std;

namespace {
	void CreateParticle(Actor* actor, NiPoint3 position, float scale) {
		auto profiler = Profilers::Profile("Explosions: CreateParticle");
		
		if (HighHeelManager::IsWearingHH(actor)) {
			SpawnParticle(actor, 4.60, "GTS/Effects/Footstep_High_Heel.nif", NiMatrix3(), position, scale * 2.9, 7, nullptr);
			SpawnParticle(actor, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), position, scale * 2.9, 7, nullptr); // Spawn both
			return;
		} else {
			SpawnParticle(actor, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), position, scale * 2.9, 7, nullptr); // Spawn foot only
			return;
		}
	}

	void make_explosion_at(FootEvent kind, Actor* actor, NiPoint3 position, float scale) {
		if (!actor) {
			return;
		}
		switch (kind) {
			case FootEvent::Left:
			case FootEvent::Right:
			case FootEvent::Front:
			case FootEvent::Back:
				CreateParticle(actor, position, scale);
			break;
			case FootEvent::JumpLand:
				CreateParticle(actor, position, scale);
			break;
			case FootEvent::Butt:
				CreateParticle(actor, position, scale);	
			break;
		}
	}
}

namespace Gts {
	ExplosionManager& ExplosionManager::GetSingleton() noexcept {
		static ExplosionManager instance;
		return instance;
	}

	std::string ExplosionManager::DebugName() {
		return "ExplosionManager";
	}

	void ExplosionManager::OnImpact(const Impact& impact) {
		if (!impact.actor) {
			return;
		}
		auto profiler = Profilers::Profile("Explosions: OnImpact");
		auto actor = impact.actor;

		float scale = impact.scale;
		float minimal_size = 2.0;
		
		if (actor->formID == 0x14) {
			if (HasSMT(actor)) {
				minimal_size = 1.0;
				scale += 0.33;
			}
		}
		if (scale > minimal_size && !actor->AsActorState()->IsSwimming()) {
			if (actor->AsActorState()->IsSprinting()) {
				scale *= 1.25; // Sprinting makes you seem bigger
				if (Runtime::HasPerk(actor, "LethalSprint")) {
					scale *= 1.75; // A lot bigger
				}
			}
			if (actor->AsActorState()->IsWalking()) {
				scale *= 0.75; // Walking makes you seem smaller
			}
			if (actor->IsSneaking()) {
				scale *= 0.60; // Sneaking makes you seem smaller
			}

			FootEvent foot_kind = impact.kind;
			
			if (foot_kind == FootEvent::JumpLand) {
				float fallmod = 1.0 + (GetFallModifier(actor) - 1.0);
				scale *= 2.25 * fallmod; // Jumping makes you sound bigger
			}
			if (HighHeelManager::IsWearingHH(actor)) {
				scale *= 1.0 + GetHighHeelsBonusDamage(actor) * 2.5; // Wearing High Heels makes explosions bigger based on HH height
			}

			for (NiAVObject* node: impact.nodes) {
				// First try casting a ray
				bool success = false;
				NiPoint3 foot_location = node->world.translate;

				float hh_offset = HighHeelManager::GetBaseHHOffset(actor).Length();
				NiPoint3 ray_start = foot_location + NiPoint3(0.0, 0.0, (20.0 * scale)); // Start a bit higher
				NiPoint3 ray_direction(0.0, 0.0, -1.0);
				float ray_length = (hh_offset + 60.0) * scale;
				NiPoint3 explosion_pos = CastRayStatics(actor, ray_start, ray_direction, ray_length, success);

				scale *= 1.0 + (Potion_GetMightBonus(actor) * 0.5);

				//log::info("Explosion success: {}", success);

				if (!success) {
					explosion_pos = foot_location;
					explosion_pos.z = actor->GetPosition().z;
					if (foot_kind == FootEvent::Butt) {
						explosion_pos.z -= 3.0 * scale;
					}
				}
				if (actor->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects")) {
					make_explosion_at(impact.kind, actor, explosion_pos, scale);
				}
				if (actor->formID != 0x14 && Runtime::GetBool("NPCSizeEffects")) {
					make_explosion_at(impact.kind, actor, explosion_pos, scale);
				}
			}
		}
	}
}
