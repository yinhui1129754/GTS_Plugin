#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/impact.hpp"
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

		auto actor = impact.actor;
		auto player = PlayerCharacter::GetSingleton();
		auto& persist = Persistent::GetSingleton();

		float tremor_scale;

		if (actor->formID != 0x14) {
			float sizedifference = ((get_visual_scale(actor)/get_visual_scale(player)) * 0.20);
			tremor_scale = persist.npc_tremor_scale * (sizedifference + 0.50);
		}
		if (actor->formID == 0x14) {
			tremor_scale = persist.tremor_scale;// * (0.95 + get_visual_scale(actor) * 0.025);
		}

		if (tremor_scale < 1e-5) {
			return;
		}

		float TinyCalamity = 0.0;
		float Modify = 1.0;
		if (HasSMT(actor)) {
			TinyCalamity = 2.2;
			Modify = 0.88;
		}

		float scale = impact.scale + TinyCalamity;

		if (!actor->AsActorState()->IsSwimming()) {
			if (actor->AsActorState()->IsSprinting()) {
				scale *= 1.25 * Modify; // Sprinting makes you seem bigger
			}
			if (actor->AsActorState()->IsWalking()) {
				scale *= 0.80; // Walking makes you seem quieter
			}
			if (actor->IsSneaking()) {
				scale *= 0.65; // Sneaking makes you seem quieter
			}
			FootEvent foot_kind = impact.kind;
			if (foot_kind == FootEvent::JumpLand) {
				scale *= 1.6 * Modify; // Jumping makes you seem bigger
			}

			if (HighHeelManager::IsWearingHH(actor) && Runtime::HasPerkTeam(actor, "hhBonus")) {
				scale *= 1.1;
			}

			for (NiAVObject* node: impact.nodes) {
				float distance = 0.0;
				if (actor->formID == 0x14) {
					distance = unit_to_meter(get_distance_to_camera(node));
				} else {
					auto point_a = node->world.translate;
					auto point_b = PlayerCharacter::GetSingleton()->GetPosition();
					auto delta = point_a - point_b;

					distance = unit_to_meter(delta.Length());
				}

				// Camera shakes
				auto config = Config::GetSingleton().GetTremor();

				float falloff = falloff_calc(distance, config.GetHalfScale());

				float min_shake_scale = config.GetMinScale(); // Before this no shaking
				float max_shake_scale = config.GetMaxScale(); // After this we have full power shaking
				float power_at_min = config.GetPowerAtMin(); // Power at minimum scale and zero distance will be much lower than this at 2m due to falloff
				float power_at_max = config.GetPowerAtMax(); // Power at maximum scale

				if (scale < min_shake_scale) {
					return;
				}
				float power = 0.0;


				// The equation to use
				//
				// FullTesting graph: https://www.desmos.com/calculator/qazgd0awcx
				std::map<std::string, Formula> s_mapStringToForumlua =
				{
					{ "linear", Formula::Linear },
					{ "smoothstep", Formula::Smooth },
					{ "softcore", Formula::SoftCore },
					{ "cubic", Formula::Power }
				};
				Formula formula = s_mapStringToForumlua[config.GetMethod()];

				switch (formula) {
					case Formula::Power:
					{
						// Power increases cubically with scale (linearly with volume)
						float n = 3.0;
						float k = 1.0/pow(max_shake_scale - min_shake_scale, n);
						power = k*pow(scale - min_shake_scale, n)*(power_at_max-power_at_min) + power_at_min;
						break;
					}
					case Formula::Smooth:
					{
						// Smooth step
						power = smootherstep(min_shake_scale, max_shake_scale, scale)*(power_at_max-power_at_min) + power_at_min;
						break;
					}
					case Formula::SoftCore:
					{
						// A root like softpower
						// https://www.desmos.com/calculator/p7vfatfljg
						float n = 0.24;
						float s = 1.0;
						float a = -1.17;
						// Altered to maintain the powerat/min/max
						float o = -pow(pow(power_at_min-a, s)-1, 1/(n*s))*(max_shake_scale-min_shake_scale)/pow(pow(power_at_max-a, s)-1.0, 1/(n*s))+min_shake_scale;
						float k = pow(pow(power_at_max-a, s)-1.0,1/(n*s))/(max_shake_scale-o);
						SoftPotential softness {
							.k = k,
							.n = n,
							.s = s,
							.o = o,
							.a = a,
						};

						power = soft_power(scale, softness);
						break;
					}
					case Formula::Linear:
					{
						// Linear
						float m = (power_at_max-power_at_min)/(max_shake_scale-min_shake_scale);
						float c = power_at_min-(power_at_max-power_at_min)/(max_shake_scale-min_shake_scale)*min_shake_scale;
						power = m*scale + c;
						break;
					}
					default: {
						log::error("Tremor method invalid");
						return;
						break;
					}
				}

				float intensity = power * falloff * tremor_scale;

				float duration = power * tremor_scale * 0.5;
				duration = smootherstep(0.2, 1.2, duration);

				bool pcEffects = Runtime::GetBoolOr("PCAdditionalEffects", true);

				if (actor->formID == 0x14 && pcEffects) {

					if (intensity > 0.01 && duration > 0.01) {
						intensity *= 1.65; // stronger PC shake
						if (IsFirstPerson()) {
							intensity *= 0.075; // Shake effects are weaker when in first person
						}
						//log::info("Shaking camera for: {}, Intensity: {}, Duration: {}, scale: {}, tremor_scale: {}", actor->GetDisplayFullName(), intensity, duration, scale, tremor_scale);
						shake_camera(actor, intensity, duration);

						float left_shake = intensity;
						float right_shake = intensity;
						if (actor->formID == 0x14) {
							switch (foot_kind) {
								case FootEvent::Left:
								case FootEvent::Front:
									right_shake = 0.0;
									break;
								case FootEvent::Right:
								case FootEvent::Back:
									left_shake = 0.0;
									break;
							}
						}
						shake_controller(left_shake, right_shake, duration);
					}
				}

				bool npcEffects = Runtime::GetBoolOr("NPCSizeEffects", true);
				if (actor->formID != 0x14 && npcEffects) {
					if (intensity > 0.01 && duration > 0.01) {
						intensity *= 1.4;
						shake_camera(actor, intensity, duration);

						float left_shake = intensity;
						float right_shake = intensity;
						if (actor->formID == 0x14) {
							switch (foot_kind) {
								case FootEvent::Left:
								case FootEvent::Front:
									right_shake = 0.0;
									break;
								case FootEvent::Right:
								case FootEvent::Back:
									left_shake = 0.0;
									break;
							}
						}
						shake_controller(left_shake, right_shake, duration);
					}
				}
			}
		}
	}
}
