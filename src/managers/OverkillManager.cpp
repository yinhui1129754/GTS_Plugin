#include "managers/OverkillManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
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
    void PlayGoreEffects(Actor* tiny, Actor* giant) {
        if (!IsLiving(tiny)) {
            SpawnDustParticle(tiny, giant, "NPC Root [Root]", 3.0);
        } else {
            if (!LessGore()) {
                auto root = find_node(tiny, "NPC Root [Root]");
                if (root) {
                    float currentSize = get_visual_scale(tiny);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, currentSize * 12.5, 7, root);
                }
            }
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::CreateExplosion(tiny, get_visual_scale(tiny) * 0.5, "BloodExplosion");
        }
    }

    void MoveItems(ActorHandle giantHandle, ActorHandle tinyHandle, FormID ID) {
        std::string taskname = std::format("Overkill {}", ID);
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
            TransferInventory(tiny, giant, scale, false, true, DamageSource::Overkill, true);
            // ^ transferInventory>TransferInventoryToDropBox also plays crush audio on loot pile
            // Works like that because Audio very often disappears on actors, so it's easier to play it on the object
        });
    }
}

namespace Gts {
	OverkillManager& OverkillManager::GetSingleton() noexcept {
		static OverkillManager instance;
		return instance;
	}

	std::string OverkillManager::DebugName() {
		return "OverkillManager";
	}

	void OverkillManager::Update() {
		auto profiler = Profilers::Profile("Overkill: Update");
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

			if (data.state == OverkillState::Healthy) {
				SetReanimatedState(tiny);
				data.state = OverkillState::Overkilling;
			} else if (data.state == OverkillState::Overkilling) {
				if (data.delay.ShouldRun()) {
                    if (!tiny->IsDead()) {
                        KillActor(giant, tiny);
                    }

                    ActorHandle giantHandle = giant->CreateRefHandle();
                    ActorHandle tinyHandle = tiny->CreateRefHandle();

                    PlayGoreEffects(tiny, giant);    
                    MoveItems(giantHandle, tinyHandle, tiny->formID);
                    PrintDeathSource(giant, tiny, DamageSource::Overkill);

                    if (tiny->formID != 0x14) {
                        Disintegrate(tiny, true); // Set critical stage 4 on actors
                    } else if (tiny->formID == 0x14) {
                        TriggerScreenBlood(50);
                        tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
                    }

                    data.state = OverkillState::Overkilled;
					Attacked(tiny, giant);
				}
			}
		}
	}


	void OverkillManager::Reset() {
		this->data.clear();
	}

	void OverkillManager::ResetActor(Actor* actor) {
		if (actor) {
			this->data.erase(actor->formID);
		}
	}

	void OverkillManager::Overkill(Actor* giant, Actor* tiny) {
		if (!tiny) {
			return;
		}
		if (!giant) {
			return;
		}
		if (OverkillManager::CanOverkill(giant, tiny)) {
			OverkillManager::GetSingleton().data.try_emplace(tiny->formID, giant);
		}
	}

	bool OverkillManager::AlreadyOverkilled(Actor* actor) {
		if (!actor) {
			return false;
		}
		auto& m = OverkillManager::GetSingleton().data;
		return !(m.find(actor->formID) == m.end());
	}

	bool OverkillManager::CanOverkill(Actor* giant, Actor* tiny) {
		if (OverkillManager::AlreadyOverkilled(tiny)) {
			return false;
		}
		if (IsEssential(tiny)) {
			return false;
		}

		return true;
	}

	OverkillData::OverkillData(Actor* giant) :
		delay(Timer(0.01)),
		state(OverkillState::Healthy),
		giant(giant ? giant->CreateRefHandle() : ActorHandle()) {
	}
}
