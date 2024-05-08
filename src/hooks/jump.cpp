#include "managers/damage/LaunchObject.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "scale/modscale.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"
#include "hooks/jump.hpp"

using namespace RE;
using namespace SKSE;

namespace {
	const float launch_up_radius = 24.0f;

	void Jump_ApplyExtraJumpEffects(Actor* actor, float size, float Might) {
		if (!actor->IsInMidair()) {
			NiPoint3 pos = actor->GetPosition(); 
			pos.z += 4.0; //shift it up a little

			if (HasSMT(actor)) {
				size += 2.8;
			}

			float calc_radius = ((54.0 / 3.0) * size) - 54.0;
			float stagger_radius = std::clamp(calc_radius, 0.0f, 54.0f); // Should start to appear at the scale of x3.0

			if (stagger_radius > 1.0) {
				
				float power = stagger_radius / 54;

				std::vector<NiPoint3> position = {
					pos,
				};

				SpawnParticle(actor, 6.00, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), pos, size * power * 2.0, 7, nullptr);
				PushObjectsUpwards(actor, position, stagger_radius * size * Might * power, 1.25 * power, true); // Launch cabbages and stuff up
				StaggerActor_Around(actor, stagger_radius * Might, true); // Launch actors up, Radius is scaled inside the function

				log::info("Jump Power: {}", power);
				log::info("Jump Radius: {}", stagger_radius);

				Rumbling::Once("MassiveJump", actor, Rumble_Default_MassiveJump * power * Might, 0.035 * power);
			}
		}
	}
}

namespace Hooks {

	void Hook_Jumping::Hook(Trampoline& trampoline) {


		static FunctionHook<float(bhkCharacterController* a_this)> GetFallDistance(
			REL::RelocationID(76430, 78269),
			[](auto* a_this){
			float result = GetFallDistance(a_this);
			auto actor = GetCharContActor(a_this);
			if (actor) {
				if (actor->formID == 0x14) {// Apply to Player only
					float scale = std::clamp(get_giantess_scale(actor), 1.0f, 99999.0f);
					if (scale > 1e-4) {
						result /= scale;
					}
				}
			}

			return result;
			}
		);

		// AE 1402bc7c3
		// SE 1402aa40c
		//
		// Is used in the jump anim event handler
		//
		//REL::Relocation<uintptr_t> hook{REL::RelocationID(41811, 42892)};
		//_GetScaleJumpHook = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x4d, 0x4d), GetScaleJumpHook);

		static FunctionHook<bool(IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const float a_in)> SkyrimSetGraphVarFloat( 
			REL::RelocationID(32143, 32887),
			[](auto* graph, const auto& a_variableName, auto a_in) {
				if (a_variableName == "VelocityZ") {
					if (a_in < 0) {
						auto actor = skyrim_cast<Actor*>(graph);
						if (actor) {
							const float CRITICALHEIGHT = 9.70;
							const float ACTORHEIGHT = 1.82*70.0;
							const float FACTOR = 0.20;
							float scale = get_giantess_scale(actor);
							float newCriticalHeight = ACTORHEIGHT*scale*FACTOR;

							float jump_factor = pow(CRITICALHEIGHT/newCriticalHeight,0.5);
							
							a_in *= jump_factor;
						}
					}
				} 
				return SkyrimSetGraphVarFloat(graph, a_variableName, a_in);
			}
		);

		static CallHook<float(Actor* actor)> SkyrimJumpHeight(RELOCATION_ID(36271, 37257),  REL::Relocate(0x190, 0x17F),
			// SE: find offset : 0x1405d2110 - 0x1405d1f80  
			// So offset is = 0x190 .  36271 = 5D1F80
			[](auto* actor) {
				float result = SkyrimJumpHeight(actor);
				//log::info("Original jump height: {}", result);
				if (actor) {
					if (actor->formID == 0x14) {
						float size = get_giantess_scale(actor);
						float might = 1.0 + Potion_GetMightBonus(actor);
						float modifier = size * might; // Compensate it, since SetScale() already boosts jump height by default
						float scaled = std::clamp(modifier, 1.0f, 99999.0f); // Can't have smaller jump heigh than x1.0

						Jump_ApplyExtraJumpEffects(actor, size, might); // Push items and actors, spawn dust ring and shake the ground

						result *= scaled / game_getactorscale(actor);
					}
				}
				return result;
			}
		);
	}
}
