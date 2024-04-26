#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/impact.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "node.hpp"


using namespace SKSE;
using namespace RE;
using namespace Gts;
using namespace std;

namespace {
	enum Formula {
		Power,
		Smooth,
		SoftCore,
		Linear,
		Unknown,
	};

	float falloff_calc(float x, float half_power) {
		float n_falloff = 2.0;
		return 1/(1+pow(pow(1/0.5-1,n_falloff)*(x)/half_power,half_power));
	}
}

namespace Gts {
	TremorManager& TremorManager::GetSingleton() noexcept {
		static TremorManager instance;
		return instance;
	}

	std::string TremorManager::DebugName() {
		return "TremorManager";
	}

	void TremorManager::OnImpact(const Impact& impact) { // This Tremor is used for regular footsteps, not feet attacks
		if (!impact.actor) {
			return;
		}

		auto profiler = Profilers::Profile("Tremor: OnImpact");
		auto& persist = Persistent::GetSingleton();

		auto actor = impact.actor;
		if (actor) {
			
			float scale;
			float duration = 2.0;
			float calamity = 1.0;

			float gain = 1.75; // Normal tremor happens past this scale
			float threshold = 1.25; // tremor starts to appear past this scale
			float size = impact.scale;

			if (actor->formID == 0x14) {
				scale = persist.tremor_scale * Rumble_Default_FootWalk * 1.5;
				if (HasSMT(actor)) {
					threshold = 0.55;
					calamity = 2.0;
					gain = 0.55;
				}
			} else {
				scale = persist.npc_tremor_scale * Rumble_Default_FootWalk;
			}

			if (scale < 1e-5) {
				return;
			}

			if (!actor->AsActorState()->IsSwimming()) {
				if (actor->AsActorState()->IsSprinting()) {
					scale *= 1.35; // Sprinting makes tremor stronger
				}
				if (actor->AsActorState()->IsWalking()) {
					scale *= 0.80; // Walking makes tremor weaker
				}
				if (actor->IsSneaking()) {
					scale *= 0.75; // Sneaking makes tremor weaker
				}
				if (impact.kind == FootEvent::JumpLand) {
					scale *= Rumble_Default_JumpLand; // Jumping makes tremor stronger
					duration *= 1.6;
				}

				scale *= 1.0 + (GetHighHeelsBonusDamage(actor) * 5.0);

				for (NiAVObject* node: impact.nodes) {
					if (node) {
						if (size > threshold) {
							if (size < gain) {
								log::info("Altering power");
								scale /= (gain/size); // slowly gain power of shakes
							}
							bool pcEffects = Runtime::GetBoolOr("PCAdditionalEffects", true);
							bool npcEffects = Runtime::GetBoolOr("NPCSizeEffects", true);
							if (actor->formID == 0x14 && pcEffects) {
								ApplyShakeAtPoint(actor, scale, node->world.translate, 1.0, duration, calamity);
							} else if (actor->formID != 0x14 && npcEffects) {
								ApplyShakeAtPoint(actor, scale, node->world.translate, 1.0, duration * 1.25, 1.0);
							}
						}
					}
				}
			}
		}
	}
}
