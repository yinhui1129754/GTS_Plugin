#include "papyrus/scale.hpp"
#include "scale/scale.hpp"
#include "scale/modscale.hpp"
#include "data/persistent.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "ActionSettings.hpp"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsControl";

	void GrowTeammate(StaticFunctionTag*, float power) {
		auto casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef) {
			return;
		}
		for (auto targetRef: FindTeammates()) {
			if (!targetRef) {
				continue;
			}

			float Volume = clamp(0.20, 1.0, get_visual_scale(targetRef)/8);
			Runtime::PlaySoundAtNode("growthSound", targetRef, Volume, 1.0, "NPC Pelvis [Pelv]");
			KnockAreaEffect(targetRef, 6, 60 * get_visual_scale(targetRef));

			// Thread safe handles
			ActorHandle casterHandle = casterRef->CreateRefHandle();
			ActorHandle targetHandle = targetRef->CreateRefHandle();

			const float DURATION = 2.0;
			std::string name = std::format("GrowFollower_{}", targetRef->formID);

			TaskManager::RunFor(name, DURATION, [=](auto& progressData){
				if (!casterHandle) {
					return false;
				}
				if (!targetHandle) {
					return false;
				}
				float timeDelta = progressData.delta * 60; // Was optimised as 60fps

				auto target = targetHandle.get().get();
				auto caster = casterHandle.get().get();

				float target_scale = get_target_scale(target);
				float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

				float bonus = 1.0;
				if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
					bonus = target_scale * 0.25 + 0.75;
				}

				DamageAV(caster, ActorValue::kMagicka, 0.45 * (target_scale * 0.25 + 0.75) * magicka * bonus * timeDelta * power);
				Grow(target, 0.0030 * magicka * bonus, 0.0);
				GRumble::Once("GrowOtherButton", target, 1.0, 0.05);

				return true;
			});
		}
	}

	void ShrinkTeammate(StaticFunctionTag*, float power) {
		auto casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef) {
			return;
		}
		for (auto targetRef: FindTeammates()) {
			if (!targetRef) {
				continue;
			}
			float scale = get_visual_scale(targetRef);
			float Volume = clamp(0.10, 1.0, scale * 0.10);
			
			Runtime::PlaySound("shrinkSound", targetRef, Volume, 1.0);
			

			// Thread safe handles
			ActorHandle casterHandle = casterRef->CreateRefHandle();
			ActorHandle targetHandle = targetRef->CreateRefHandle();

			const float DURATION = 2.0;
			std::string name = std::format("ShrinkFollower_{}", targetRef->formID);

			TaskManager::RunFor(name, DURATION, [=](auto& progressData){
				if (!casterHandle) {
					return false;
				}
				if (!targetHandle) {
					return false;
				}

				auto target = targetHandle.get().get();
				auto caster = casterHandle.get().get();

				float target_scale = get_target_scale(target);
				float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

				float bonus = 1.0;
				if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
					bonus = target_scale * 0.25 + 0.75;
				}

				if (target_scale > get_natural_scale(target)) {
					DamageAV(caster, ActorValue::kMagicka, 0.25 * (target_scale * 0.25 + 0.75) * magicka * bonus * TimeScale() * power);
					ShrinkActor(target, 0.0030 * magicka * bonus, 0.0);
					GRumble::Once("ShrinkOtherButton", target, 1.0, 0.05);
				}
				return true;
			});
		}
	}

	void GrowPlayer(StaticFunctionTag*, float power) {
		auto casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef) {
			return;
		}

		float scale = get_visual_scale(casterRef);
		float Volume = clamp(0.20, 1.0, scale * 0.20);

		KnockAreaEffect(casterRef, 6, 60 * scale);
		Runtime::PlaySoundAtNode("growthSound", casterRef, Volume, 1.0, "NPC Pelvis [Pelv]");
	
		// Thread safe handles
		ActorHandle casterHandle = casterRef->CreateRefHandle();

		const float DURATION = 2.0;
		std::string name = std::format("GrowPlayer_{}", casterRef->formID);

		TaskManager::RunFor(name, DURATION, [=](auto& progressData){
			if (!casterHandle) {
				return false;
			}

			auto caster = casterHandle.get().get();

			float caster_scale = get_visual_scale(caster);
			float target_scale = get_target_scale(caster);

			float bonus = 1.0;
			if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
				bonus = target_scale * 0.25 + 0.75;
			}

			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
			DamageAV(caster, ActorValue::kStamina, 0.45 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());

			Grow(caster, 0.0030 * stamina, 0.0);

			GRumble::Once("GrowButton", caster, 1.0, 0.05);

			return true;
		});
	}
	

	void ShrinkPlayer(StaticFunctionTag*, float power) {
		auto casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef) {
			return;
		}

		float scale = get_visual_scale(casterRef);
		float Volume = clamp(0.10, 1.0, scale * 0.10);

		Runtime::PlaySound("shrinkSound", casterRef, Volume, 1.0);
	
		// Thread safe handles
		ActorHandle casterHandle = casterRef->CreateRefHandle();

		const float DURATION = 2.0;
		std::string name = std::format("ShrinkPlayer_{}", casterRef->formID);

		TaskManager::RunFor(name, DURATION, [=](auto& progressData){
			if (!casterHandle) {
				return false;
			}

			auto caster = casterHandle.get().get();

			float caster_scale = get_visual_scale(caster);
			float target_scale = get_target_scale(caster);

			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

			float bonus = 1.0;
			if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
				bonus = target_scale * 0.25 + 0.75;
			}

			if (target_scale > Minimum_Actor_Scale) {
				DamageAV(caster, ActorValue::kStamina, 0.25 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());
				ShrinkActor(caster, 0.0020 * stamina, 0.0);
				GRumble::Once("ShrinkButton", caster, 0.60, 0.05);
			} else {
				set_target_scale(caster, Minimum_Actor_Scale);
				return false;
			}
			return true;
		});
	}

	void CallRapidGrowth(StaticFunctionTag*, float amt, float halflife) {
		auto PC = PlayerCharacter::GetSingleton();
		SpringGrow(PC, amt, halflife, "Input");
	}

	void CallRapidShrink(StaticFunctionTag*, float amt, float halflife) {
		auto PC = PlayerCharacter::GetSingleton();
		SpringShrink(PC, amt, halflife, "Input");
	}

}

namespace Gts {
	bool register_total_control(IVirtualMachine* vm) {
		vm->RegisterFunction("GrowTeammate", PapyrusClass, GrowTeammate);
		vm->RegisterFunction("ShrinkTeammate", PapyrusClass, ShrinkTeammate);

		vm->RegisterFunction("GrowPlayer", PapyrusClass, GrowPlayer);
		vm->RegisterFunction("ShrinkPlayer", PapyrusClass, ShrinkPlayer);

		vm->RegisterFunction("CallRapidGrowth", PapyrusClass, CallRapidGrowth);
		vm->RegisterFunction("CallRapidShrink", PapyrusClass, CallRapidShrink);

		return true;
	}

	
}