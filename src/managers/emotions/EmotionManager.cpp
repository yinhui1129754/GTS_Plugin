#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"

namespace {

	const float Speed_up = 12.0f;
	const double LAUGH_COOLDOWN = 5.0f;
	const double MOAN_COOLDOWN = 5.0f;

	BSFaceGenAnimationData* GetFacialData(Actor* giant) {
		auto fgen = giant->GetFaceGenAnimationData();
		if (fgen) {
			return fgen;
		}
		return nullptr;
	}

	float Phenome_GetPhenomeValue(BSFaceGenAnimationData* data, std::uint32_t Phenome) {
		float value = data->phenomeKeyFrame.values[Phenome];
		return value;
	}

	float Phenome_GetModifierValue(BSFaceGenAnimationData* data, std::uint32_t Modifier) {
		float value = data->modifierKeyFrame.values[Modifier];
		return value;
	}

	void Phenome_ManagePhenomes(BSFaceGenAnimationData* data, std::uint32_t Phenome, float Value) {
		data->phenomeKeyFrame.SetValue(Phenome, Value);
	}

	void Phenome_ManageModifiers(BSFaceGenAnimationData* data, std::uint32_t Modifier, float Value) {
		data->modifierKeyFrame.SetValue(Modifier, Value);
	}

	void Task_UpdatePhenome(Actor* giant, int phenome, float halflife, float target) {
		std::string name = std::format("Phenome_{}_{}_{}", giant->formID, phenome, target);
		ActorHandle giantHandle = giant->CreateRefHandle();

		float modified = 0.0;
		auto data = GetFacialData(giant);
		if (data) {
			modified = Phenome_GetPhenomeValue(data, phenome);
		}
		
		float start = Time::WorldTimeElapsed();

		bool Reset = (target < 0.01);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}

			auto giantref = giantHandle.get().get();
			float pass = Time::WorldTimeElapsed() - start;

			if (!giantref->Is3DLoaded()) {
				return false;
			}

			float AnimSpeed = AnimationManager::GetSingleton().GetAnimSpeed(giant);
			float speed = 1.25 * AnimSpeed * halflife * Speed_up;

			float value = (pass * speed);
			auto FaceData = GetFacialData(giantref);
			if (FaceData) {
				if (Reset && modified != 0.0) {
					value = modified - (pass * speed);
					Phenome_ManagePhenomes(FaceData, phenome, value);
					if (value <= 0) {
						Phenome_ManagePhenomes(FaceData, phenome, 0.0);
						return false;
					}
					return true;
				} if (value >= target) { // fully applied
					Phenome_ManagePhenomes(FaceData, phenome, target);
					return false;
				} 

				Phenome_ManagePhenomes(FaceData, phenome, value);
				return true;
			}

			return false;
		});
	}

	void Task_UpdateModifier(Actor* giant, int modifier, float halflife, float target) {

		std::string name = std::format("Modifier_{}_{}_{}", giant->formID, modifier, target);

		float modified = 0.0;

		auto data = GetFacialData(giant);
		if (data) {
			modified = Phenome_GetModifierValue(data, modifier);
		}

		ActorHandle giantHandle = giant->CreateRefHandle();
		

		float start = Time::WorldTimeElapsed();

		bool Reset = (target < 0.01);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}

			auto giantref = giantHandle.get().get();
			float pass = Time::WorldTimeElapsed() - start;

			if (!giantref->Is3DLoaded()) {
				return false;
			}

			float AnimSpeed = AnimationManager::GetSingleton().GetAnimSpeed(giant);
			float speed = 1.0 * AnimSpeed * halflife * Speed_up;

			float value = (pass * speed);
			auto FaceData = GetFacialData(giantref);
			if (FaceData) {
				if (Reset) {
					value = modified - (pass * speed);
					Phenome_ManageModifiers(FaceData, modifier, value);
					if (value <= 0) {
						Phenome_ManageModifiers(FaceData, modifier, 0.0);
						return false;
					}
					return true;
				} if (value >= target) { // fully applied
					Phenome_ManageModifiers(FaceData, modifier, target);
					return false;
				} 

				Phenome_ManageModifiers(FaceData, modifier, value);
				return true;
			}

			return false;
		});
	}
}

namespace Gts {

	EmotionManager& EmotionManager::GetSingleton() noexcept {
		static EmotionManager instance;
		return instance;
	}

	std::string EmotionManager::DebugName() {
		return "EmotionManager";
	}

	void EmotionManager::Reset() {
		this->EmotionData.clear();
	}

	void EmotionManager::OverridePhenome(Actor* giant, int number, float power, float halflife, float target) {
		Task_UpdatePhenome(giant, number, halflife, target);
	}

	void EmotionManager::OverrideModifier(Actor* giant, int number, float power, float halflife, float target) {
		Task_UpdateModifier(giant, number, halflife, target);
	}

	bool EmotionManager::Laugh_InCooldown(Actor* actor) {
		return Time::WorldTimeElapsed() <= (EmotionManager::GetSingleton().GetEmotionData(actor).lastLaughTime + LAUGH_COOLDOWN);
	}

	bool EmotionManager::Moan_InCooldown(Actor* actor) {
		return Time::WorldTimeElapsed() <= (EmotionManager::GetSingleton().GetEmotionData(actor).lastMoanTime + MOAN_COOLDOWN);
	}

	EmotionData& EmotionManager::GetEmotionData(Actor* actor) {
		this->EmotionData.try_emplace(actor);
		return this->EmotionData.at(actor);
	}
}