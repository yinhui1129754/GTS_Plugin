#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "utils/looting.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "node.hpp"

#include <random>

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {

	void CrushGrow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float modifier = SizeManager::GetSingleton().BalancedMode();
		scale_factor /= modifier;
		bonus /= modifier;
		update_target_scale(actor, CalcPower(actor, scale_factor, bonus, false), SizeEffectType::kGrow);
		AddStolenAttributes(actor, CalcPower(actor, scale_factor, bonus, false));
	}

	void ScareChance(Actor* actor) {
		int voreFearRoll = rand() % 5;
		if (HasSMT(actor)) {
			voreFearRoll = rand() % 2;
			shake_camera(actor, 0.4, 0.25);
		}

		if (voreFearRoll <= 0) {
			Runtime::CastSpell(actor, actor, "GtsVoreFearSpell");
			KnockAreaEffect(actor, 6, 60 * get_visual_scale(actor));
		}
	}

	void ProgressQuest(Actor* giant, Actor* tiny) {
		if (!tiny->IsDead()) {
			if (IsGiant(tiny)) {
				AdvanceQuestProgression(giant, tiny, 7, 1, false);
			} else {
				AdvanceQuestProgression(giant, tiny, 3, 1, false);
			}
		} else {
			AdvanceQuestProgression(giant, tiny, 3, 0.25, false);
		}
	}

	void FearChance(Actor* giant)  {
		float size = get_visual_scale(giant);
		int MaxValue = (20 - (1.6 * size));

		if (MaxValue <= 3 || HasSMT(giant)) {
			MaxValue = 3;
		}
		int FearChance = rand() % MaxValue;
		if (FearChance <= 0) {
			Runtime::CastSpell(giant, giant, "GtsVoreFearSpell");
			// Should cast fear
		}
	}

	void PleasureText(Actor* actor) {
		int Pleasure = rand() % 5;
		if (Pleasure <= 0) {
			if (actor->formID == 0x14) {
				Notify("Crushing your foes feels good and makes you bigger");
			} else {
				Notify("Your companion grows bigger by crushing your foes");
			}
		}
	}

	void GrowAfterTheKill(Actor* caster, Actor* target) {
		if (!Runtime::GetBool("GtsDecideGrowth") || HasSMT(caster)) {
			return;
		} else if (Runtime::HasPerkTeam(caster, "GrowthDesirePerk") && Runtime::GetInt("GtsDecideGrowth") >= 1) {
			float Rate = (0.00016 * get_visual_scale(target)) * 120;
			if (Runtime::HasPerkTeam(caster, "AdditionalGrowth")) {
				Rate *= 2.0;
			}
			if (Runtime::HasPerkTeam(caster, "AdditionalGrowth_p2")) {
				Rate *= 1.6;
				caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, 25.0);
			}
			CrushGrow(caster, 0, Rate * SizeSteal_GetPower(caster, target));
		}
		PleasureText(caster);
	}
	void MoanOrLaugh(Actor* giant, Actor* target) {
		static Timer voicetimer = Timer(2.4);
		auto randomInt = rand() % 16;
		auto select = rand() % 3;
		if (randomInt <= 3.0) {
			if (voicetimer.ShouldRun()) {
				if (select >= 2.0) {
					PlayMoanSound(giant, 1.0);
					GrowAfterTheKill(giant, target);
					Task_FacialEmotionTask_Moan(giant, 2.0, "Crush");
				} else {
					PlayLaughSound(giant, 1.0, 2);
				}
			}
		}
	}
}

namespace Gts {
	CrushManager& CrushManager::GetSingleton() noexcept {
		static CrushManager instance;
		return instance;
	}

	std::string CrushManager::DebugName() {
		return "CrushManager";
	}

	void CrushManager::Update() {
		auto profiler = Profilers::Profile("CrushManager: Update");
		for (auto &[tinyId, data]: this->data) {
			auto tiny = TESForm::LookupByID<Actor>(tinyId);
			auto giantHandle = data.giant;
			if (!tiny) {
				continue;
			}
			if (!giantHandle) {
				continue;
			}
			auto giant = giantHandle.get().get();
			if (!giant) {
				continue;
			}

			auto transient = Transient::GetSingleton().GetData(tiny);
			if (transient) {
				if (!transient->can_be_crushed && !tiny->IsDead()) {
					return;
				}
			}

			if (data.state == CrushState::Healthy) {
				SetReanimatedState(tiny);
				ProgressQuest(giant, tiny);
				data.state = CrushState::Crushing;
			} else if (data.state == CrushState::Crushing) {
				Attacked(tiny, giant);

				float currentSize = get_visual_scale(tiny);

				data.state = CrushState::Crushed;
				if (giant->formID == 0x14 && IsDragon(tiny)) {
					CompleteDragonQuest(tiny, false, tiny->IsDead());
				}
				
				std::string taskname = std::format("CrushTiny {}", tiny->formID);

				MoanOrLaugh(giant, tiny);
				GrowAfterTheKill(giant, tiny);

				GRumble::Once("CrushRumble", tiny, 1.4, 0.15);
				if (giant->formID == 0x14) {
					if (IsLiving(tiny)) {
						TriggerScreenBlood(50);
					}
				}
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> dis(-0.2, 0.2);

				AddSMTDuration(giant, 5.0);
				ScareChance(giant);

				// Do crush
				KillActor(giant, tiny);

				if (!IsLiving(tiny) || LessGore()) {
					SpawnDustParticle(giant, tiny, "NPC Root [Root]", 3.0);
				} else {
					if (!LessGore()) {
						auto root = find_node(tiny, "NPC Root [Root]");
						if (root) {
							SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
							SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
							SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
							SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
							SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, currentSize * 25, 7, root);
						}
						Runtime::CreateExplosion(tiny, get_visual_scale(tiny)/4,"BloodExplosion");
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
					}
				}
				ActorHandle giantHandle = giant->CreateRefHandle();
				ActorHandle tinyHandle = tiny->CreateRefHandle();
				TaskManager::RunOnce(taskname, [=](auto& update){
					if (!tinyHandle) {
						return;
					}
					if (!giantHandle) {
						return;
					}

					auto giant = giantHandle.get().get();
					auto tiny = tinyHandle.get().get();
					float scale = get_visual_scale(tiny);
					TransferInventory(tiny, giant, scale, false, true, DamageSource::Crushed, true);
					// Actor Reset is done inside TransferInventory:StartResetTask!
				});

				if (tiny->formID != 0x14) {
					Disintegrate(tiny, true); // Set critical stage 4 on actors
				} else if (tiny->formID == 0x14) {
					TriggerScreenBlood(50);
					tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
				}

				FearChance(giant);
			}
		}
	}

	void CrushManager::Reset() {
		this->data.clear();
	}

	void CrushManager::ResetActor(Actor* actor) {
		if (actor) {
			this->data.erase(actor->formID);
		}
	}

	void CrushManager::Crush(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (CrushManager::CanCrush(giant, tiny)) {
			CrushManager::GetSingleton().data.try_emplace(tiny->formID, giant);
		}
	}

	bool CrushManager::AlreadyCrushed(Actor* actor) {
		if (!actor) {
			return false;
		}
		auto& m = CrushManager::GetSingleton().data;
		return (m.find(actor->formID) != m.end());
	}

	bool CrushManager::CanCrush(Actor* giant, Actor* tiny) {
		if (CrushManager::AlreadyCrushed(tiny)) {
			return false;
		}
		if (IsEssential(tiny)) {
			return false;
		}
		// Check if they are immune
		const std::string_view CANT_CRUSH_EDITORID = "GtsCantStomp";
		if (tiny->HasKeywordString(CANT_CRUSH_EDITORID)) {
			// TODO: Check GtsCantStomp is a valid keyword
			return false;
		}
		//Check for Essential

		// Check skin
		auto skin = tiny->GetSkin();
		if (skin) {
			if (skin->HasKeywordString(CANT_CRUSH_EDITORID)) {
				return false;
			}
		}
		const auto inv = tiny->GetInventory([](TESBoundObject& a_object) {
			return a_object.IsArmor();
		});

		// Check worn armor
		for (const auto& [item, invData] : inv) {
			const auto& [count, entry] = invData;
			if (count > 0 && entry->IsWorn()) {
				const auto armor = item->As<TESObjectARMO>();
				if (armor && armor->HasKeywordString(CANT_CRUSH_EDITORID)) {
					return false;
				}
			}
		}
		//log::info("Can crush {}", tiny->GetDisplayFullName());
		return true;
	}

	CrushData::CrushData(Actor* giant) :
		delay(Timer(0.01)),
		state(CrushState::Healthy),
		giant(giant ? giant->CreateRefHandle() : ActorHandle()) {
	}
}
