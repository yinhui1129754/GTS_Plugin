#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/MovementForce.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/highheel.hpp"
#include "managers/explosion.hpp"
#include "managers/audio/footstep.hpp"
#include "utils/DeathReport.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "utils/looting.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;


namespace {
	std::string_view GetImpactNode(CrawlEvent kind) {
		if (kind == CrawlEvent::RightKnee) {
			return "NPC R Calf [RClf]";
		} else if (kind == CrawlEvent::LeftKnee) {
			return "NPC L Calf [LClf]";
		} else if (kind == CrawlEvent::RightHand) {
			return "NPC R Finger20 [RF20]";
		} else if (kind == CrawlEvent::LeftHand) {
			return "NPC L Finger20 [LF20]";
		} else {
			return "NPC L Finger20 [LF20]";
		}
	}

	float GetStaggerThreshold(DamageSource Cause) {
		float StaggerThreshold = 1.0;
		if (Cause == DamageSource::HandSwipeRight || Cause == DamageSource::HandSwipeLeft) {
			StaggerThreshold = 1.4; // harder to stagger with hand swipes
		}
		return StaggerThreshold;
	}
}

namespace Gts {

	const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "NPC L Toe0 [LToe]";
	const std::string_view rightToeLookup = "NPC R Toe0 [RToe]";
	const std::string_view bodyLookup = "NPC Spine1 [Spn1]";

	void BlockFirstPerson(Actor* actor, bool block) { // Credits to ArranzCNL for this function. Forces Third Person because we don't have FP working yet.
		auto playerControls = RE::PlayerControls::GetSingleton();
		auto camera = RE::PlayerCamera::GetSingleton();
		auto controlMap = RE::ControlMap::GetSingleton();
		if (block) {
			controlMap->enabledControls.reset(RE::UserEvents::USER_EVENT_FLAG::kPOVSwitch); // Block POV Switching
			camera->ForceThirdPerson();
			return;
		}
		//playerControls->data.povScriptMode = block;
		controlMap->enabledControls.set(RE::UserEvents::USER_EVENT_FLAG::kPOVSwitch); // Allow POV Switching
	}

	void Hugs_FixAnimationDesync(Actor* giant, Actor* tiny, bool reset) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			float& animspeed = transient->Hug_AnimSpeed;
			if (!reset) {
				animspeed = AnimationManager::GetAnimSpeed(giant);
			} else {
				animspeed = 1.0; // 1.0 makes dll use GetAnimSpeed of tiny
			}
			// Fix hug anim de-sync
		}
	}

	void ForceFollowerAnimation(Actor* giant, FollowerAnimType Type) {
		std::size_t numberOfPrey = 1000;

		auto& Vore =        Vore::GetSingleton();
		auto& ButtCrush = 	ButtCrushController::GetSingleton();
		auto& Hugs = 		HugAnimationController::GetSingleton();
		auto& Grabs = 		GrabAnimationController::GetSingleton();
		auto& Sandwich =    ThighSandwichController::GetSingleton();

		switch (Type) {
			case FollowerAnimType::ButtCrush: {
				for (auto new_gts: FindTeammates()) {
					if (IsTeammate(new_gts)) {
						for (auto new_tiny: ButtCrush.GetButtCrushTargets(new_gts, numberOfPrey)) { 
							if (new_tiny->formID == 0x14) {
								if (ButtCrush.CanButtCrush(new_gts, new_tiny)) {
									ButtCrush.StartButtCrush(new_gts, new_tiny);
									ControlAnother(new_gts, false);
									return;
								}
							}
						}
					}
				}
			break;	
			}
		 	case FollowerAnimType::Hugs: {
				for (auto new_gts: FindTeammates()) {
					if (IsTeammate(new_gts)) {
						for (auto new_tiny: Hugs.GetHugTargetsInFront(new_gts, numberOfPrey)) { 
							if (new_tiny->formID == 0x14) {
								float sizedifference = GetSizeDifference(new_gts, new_tiny, SizeType::VisualScale, true, true);
								bool allow = (sizedifference >= Action_Hug && sizedifference < GetHugShrinkThreshold(new_gts));
								if (allow && Hugs.CanHug(new_gts, new_tiny)) {
									Hugs.StartHug(new_gts, new_tiny);
									ControlAnother(new_gts, false);
									return;
								}
							}
						}
					}
				}
			break;
			}
		 	case FollowerAnimType::Grab: {
				for (auto new_gts: FindTeammates()) {
					if (IsTeammate(new_gts)) {
						std::vector<Actor*> FindTiny = Grabs.GetGrabTargetsInFront(new_gts, numberOfPrey);
						for (auto new_tiny: FindTiny) { 
							if (new_tiny->formID == 0x14 && Grabs.CanGrab(new_gts, new_tiny)) {
								Grabs.StartGrab(new_gts, new_tiny);
								ControlAnother(new_gts, false);
								return;
							}
						}
					}
				}
				break;	
			}
		 	case FollowerAnimType::Vore: {	
				for (auto new_gts: FindTeammates()) {
					if (IsTeammate(new_gts)) {
						for (auto new_tiny: Vore.GetVoreTargetsInFront(new_gts, numberOfPrey)) { 
							if (new_tiny->formID == 0x14) {
								if (Vore.CanVore(new_gts, new_tiny)) {
									Vore.StartVore(new_gts, new_tiny);
									ControlAnother(new_gts, false);
									return;
								}
							}
						}
					}
				}
			break;
			} 
		 	case FollowerAnimType::ThighSandwich: {
				for (auto new_gts: FindTeammates()) {
					if (IsTeammate(new_gts)) {
						for (auto new_tiny: Sandwich.GetSandwichTargetsInFront(new_gts, numberOfPrey)) { 
							if (new_tiny->formID == 0x14) {
								if (Sandwich.CanSandwich(new_gts, new_tiny)) {
									Sandwich.StartSandwiching(new_gts, new_tiny);
									ControlAnother(new_gts, false);
									return;
								}
							}
						}
					}
				} 
			break;
			}
		}
	}
		
	
		 

	void Vore_AttachToRightHandTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("CrawlVore_{}_{}", giant->formID, tiny->formID);
		ActorHandle giantHandle = giant->CreateRefHandle();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			} 
			if (!tinyHandle) {
				return false;
			}
			auto giantref = giantHandle.get().get();
			auto tinyref = tinyHandle.get().get();

			auto FingerA = find_node(giant, "NPC R Finger02 [RF02]");
			if (!FingerA) {
				Notify("R Finger 02 node not found");
				return false;
			}
			auto FingerB = find_node(giant, "NPC R Finger30 [RF30]");
			if (!FingerB) {
				Notify("R Finger 30 node not found");
				return false;
			}
			NiPoint3 coords = (FingerA->world.translate + FingerB->world.translate) / 2.0;
			coords.z -= 3.0;

			if (tinyref->IsDead()) {
				Notify("Vore Task ended");
				return false;
			}

			return AttachTo(giantref, tinyref, coords);
		});
	}
	
	bool Vore_ShouldAttachToRHand(Actor* giant, Actor* tiny) {
		if (IsTransferingTiny(giant)) {
			Vore_AttachToRightHandTask(giant, tiny); // start "attach to hand" task outside of vore.cpp
			return true;
		} else {
			return false;
		}
	}

	void UpdateFriendlyHugs(Actor* giant, Actor* tiny, bool force) {
		bool hostile = IsHostile(tiny, giant);
		bool teammate = IsTeammate(tiny) || tiny->formID == 0x14;
		bool perk = Runtime::HasPerkTeam(giant, "HugCrush_LovingEmbrace");

		if (perk && !hostile && teammate && !force) {
			tiny->SetGraphVariableBool("GTS_IsFollower", true);
			giant->SetGraphVariableBool("GTS_HuggingTeammate", true);
		} else {
			tiny->SetGraphVariableBool("GTS_IsFollower", false);
			giant->SetGraphVariableBool("GTS_HuggingTeammate", false);
		}
		// This function determines the following:
		// Should the Tiny play "willing" or "Unwilling" hug idle?
	}

	void HugCrushOther(Actor* giant, Actor* tiny) {
		Attacked(tiny, giant);
		if (giant->formID == 0x14 && IsDragon(tiny)) {
			CompleteDragonQuest(tiny, ParticleType::Red, tiny->IsDead());
		}
		float currentSize = get_visual_scale(tiny);

		ModSizeExperience(giant, 0.24); // Adjust Size Matter skill
		KillActor(giant, tiny);

		if (!IsLiving(tiny)) {
			SpawnDustParticle(tiny, tiny, "NPC Root [Root]", 3.6);
		} else {
			if (!LessGore()) {
				auto root = find_node(tiny, "NPC Root [Root]");
				if (root) {
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
					SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, get_visual_scale(tiny) * 10, 7, root);
				}
				Runtime::CreateExplosion(tiny, get_visual_scale(tiny)/4, "BloodExplosion");
				Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
			} else {
				Runtime::PlaySound("BloodGushSound", tiny, 1.0, 0.5);
			}
		}

		AddSMTDuration(giant, 5.0);

		ApplyShakeAtNode(tiny, 20, "NPC Root [Root]", 20.0);

		ActorHandle giantHandle = giant->CreateRefHandle();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		std::string taskname = std::format("HugCrush {}", tiny->formID);

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
		});
		if (tiny->formID != 0x14) {
			Disintegrate(tiny, true); // Set critical stage 4 on actor
		} else {
			TriggerScreenBlood(50);
			tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
		}
		auto Node = find_node(giant, "NPC Spine2 [Spn2]"); 
		if (!Node) {
			Notify("Error: Spine2 [Spn2] node not found");
			return;
		}
		Runtime::PlaySoundAtNode("ShrinkToNothingSound", giant, 1.0, 1.0, "NPC Spine2 [Spn2]");
	}

	// Cancels all hug-related things
	void AbortHugAnimation(Actor* giant, Actor* tiny) {
		
		bool Friendly;
		giant->GetGraphVariableBool("GTS_HuggingTeammate", Friendly);

		SetSneaking(giant, false, 0);

		AdjustFacialExpression(giant, 0, 0.0, "phenome");
		AdjustFacialExpression(giant, 0, 0.0, "modifier");
		AdjustFacialExpression(giant, 1, 0.0, "modifier");

		AnimationManager::StartAnim("Huggies_Spare", giant); // Start "Release" animation on Giant
		
		log::info("Starting abort animation, friendly: {}", Friendly);

		if (Friendly) { // If friendly, we don't want to push/release actor
			AnimationManager::StartAnim("Huggies_Spare", tiny);
			return; // GTS_Hug_Release event (HugHeal.cpp) handles it instead.
		}

		if (tiny) {
			EnableCollisions(tiny);
			SetBeingHeld(tiny, false);
			PushActorAway(giant, tiny, 1.0);
			UpdateFriendlyHugs(giant, tiny, true); // set GTS_IsFollower (tiny) and GTS_HuggingTeammate (GTS) bools to false
			Hugs_FixAnimationDesync(giant, tiny, true); // reset anim speed override so .dll won't use it
		}
		HugShrink::Release(giant);
		log::info("Releasing Tiny");
		
	}

	void Utils_UpdateHugBehaviors(Actor* giant, Actor* tiny) { // blend between two anims: send value to behaviors
        float tinySize = get_visual_scale(tiny);
        float giantSize = get_visual_scale(giant);
        float size_difference = std::clamp(giantSize/tinySize, 1.0f, 3.0f);

		float OldMin = 1.0;
		float OldMax = 3.0;

		float NewMin = 0.0;
		float NewMax = 1.0;

		float OldValue = size_difference;
		float NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;

		tiny->SetGraphVariableFloat("GTS_SizeDifference", NewValue); // pass Tiny / Giant size diff POV to Tiny
		giant->SetGraphVariableFloat("GTS_SizeDifference", NewValue); // pass Tiny / Giant size diff POV to GTS
    }

	void Utils_UpdateHighHeelBlend(Actor* giant, bool reset) { // needed to blend between 2 animations so hand will go lower
		if (!reset) {
			float max_heel_height = 0.215; // All animations are configured with this value in mind. Blending isn't configured for heels bigger than this value.
			float hh_value = HighHeelManager::GetBaseHHOffset(giant)[2]/100;
			float hh_offset = std::clamp(hh_value / max_heel_height, 0.0f, 1.0f); // reach max HH at 0.215 offset (highest i've seen and the max that we support)
		
			giant->SetGraphVariableFloat("GTS_HHoffset", hh_offset);
		} else {
			giant->SetGraphVariableFloat("GTS_HHoffset", 0.0); // reset it
		}
	}

	void StartHealingAnimation(Actor* giant, Actor* tiny) {
		UpdateFriendlyHugs(giant, tiny, false);
		AnimationManager::StartAnim("Huggies_Heal", giant);

		if (IsFemale(tiny)) {
			AnimationManager::StartAnim("Huggies_Heal_Victim_F", tiny);
		} else {
			AnimationManager::StartAnim("Huggies_Heal_Victim_M", tiny);
		}
	}

	void AllowToDoVore(Actor* actor, bool toggle) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->can_do_vore = toggle;
		}
	}

	void AllowToBeCrushed(Actor* actor, bool toggle) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->can_be_crushed = toggle;
		}
	}

	void ManageCamera(Actor* giant, bool enable, CameraTracking type) {
		if (giant->formID == 0x14) {
			if (AllowCameraTracking()) {
				auto& sizemanager = SizeManager::GetSingleton();
				sizemanager.SetTrackedBone(giant, enable, type);
			}
		}
	}

	void DoLaunch(Actor* giant, float radius, float power, FootEvent kind) {
		float smt_power = 1.0;
		float smt_radius = 1.0;
		if (HasSMT(giant)) {
			smt_power *= 2.0;
			smt_radius *= 1.25;
		}
		LaunchActor::GetSingleton().ApplyLaunch_At(giant, radius * smt_radius, power * smt_power, kind);
	}

	void DoLaunch(Actor* giant, float radius, float power, NiAVObject* node) {
		float smt_power = 1.0;
		float smt_radius = 1.0;
		if (HasSMT(giant)) {
			smt_power *= 2.0;
			smt_radius *= 1.25;
		}
		LaunchActor::GetSingleton().LaunchAtObjectNode(giant, radius * smt_radius, 0.0, power * smt_power, node);
	}

	void GrabStaminaDrain(Actor* giant, Actor* tiny, float sizedifference) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		WasteMult *= Perk_GetCostReduction(giant);

		if (giant->formID != 0x14) {
			WasteMult *= 0.33; // less drain for non-player
		}

		float WasteStamina = (1.40 * WasteMult)/sizedifference * TimeScale();
		DamageAV(giant, ActorValue::kStamina, WasteStamina);
	}

	void DrainStamina(Actor* giant, std::string_view TaskName, std::string_view perk, bool enable, float power) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, perk)) {
			WasteMult -= 0.35;
		}
		WasteMult *= Perk_GetCostReduction(giant);

		std::string name = std::format("StaminaDrain_{}_{}", TaskName, giant->formID);
		if (enable) {
			ActorHandle GiantHandle = giant->CreateRefHandle();
			TaskManager::Run(name, [=](auto& progressData) {
				if (!GiantHandle) {
					return false;
				}
				auto GiantRef = GiantHandle.get().get();
				float stamina = GetAV(giant, ActorValue::kStamina);
				if (stamina <= 1.0) {
					return false; // Abort if we don't have stamina so it won't drain it forever. Just to make sure.
				}
				float multiplier = AnimationManager::GetAnimSpeed(giant);
				float WasteStamina = 0.50 * power * multiplier;
				DamageAV(giant, ActorValue::kStamina, WasteStamina * WasteMult * TimeScale());
				return true;
			});
		} else {
			TaskManager::Cancel(name);
		}
	}

	void SpawnHurtParticles(Actor* giant, Actor* grabbedActor, float mult, float dustmult) {
		auto hand = find_node(giant, "NPC L Hand [LHnd]");
		if (hand) {
			if (IsLiving(grabbedActor)) {
				if (!LessGore()) {
					SpawnParticle(giant, 25.0, "GTS/Damage/Explode.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 3* mult, 4, hand);
					SpawnParticle(giant, 25.0, "GTS/Damage/Crush.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 3 *  mult, 4, hand);
				} else if (LessGore()) {
					Runtime::PlaySound("BloodGushSound", grabbedActor, 1.0, 0.5);
				}
			} else {
				SpawnDustParticle(giant, grabbedActor, "NPC L Hand [LHnd]", dustmult);
			}
		}
	}

	void AdjustFacialExpression(Actor* giant, int ph, float target, std::string_view type) {
		auto& Emotions = EmotionManager::GetSingleton();

		if (type == "phenome") {
			Emotions.OverridePhenome(giant, ph, 0.0, 0.08, target);
		}
		if (type == "expression") {
			auto fgen = giant->GetFaceGenAnimationData();
			if (fgen) {
				//fgen->exprOverride = false;
				fgen->SetExpressionOverride(ph, target);
				fgen->expressionKeyFrame.SetValue(ph, target); // Expression doesn't need Spring since it is already smooth by default
				//fgen->exprOverride = true;
			}
		}
		if (type == "modifier") {
			Emotions.OverrideModifier(giant, ph, 0.0, 0.25, target);
		}
	} 

	void AdjustFacialExpression(Actor* giant, int ph, float target, float speed_phenome, float speed_modifier, std::string_view type) {
		auto& Emotions = EmotionManager::GetSingleton();

		if (type == "phenome") {
			Emotions.OverridePhenome(giant, ph, 0.0, speed_phenome, target);
		}
		if (type == "expression") {
			auto fgen = giant->GetFaceGenAnimationData();
			if (fgen) {
				fgen->exprOverride = false;
				fgen->SetExpressionOverride(ph, target);
				fgen->expressionKeyFrame.SetValue(ph, target); // Expression doesn't need Spring since it is already smooth by default
				fgen->exprOverride = true;
			}
		}
		if (type == "modifier") {
			Emotions.OverrideModifier(giant, ph, 0.0, speed_modifier, target);
		}
	}

	float GetWasteMult(Actor* giant) {
		float WasteMult = 1.0;
		if (Runtime::HasPerk(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		WasteMult *= Perk_GetCostReduction(giant);
		return WasteMult;
	}

	float GetPerkBonus_Basics(Actor* Giant) {
		if (Runtime::HasPerkTeam(Giant, "DestructionBasics")) {
			return 1.25;
		} else {
			return 1.0;
		}
	}

	float GetPerkBonus_Thighs(Actor* Giant) {
		if (Runtime::HasPerkTeam(Giant, "KillerThighs")) {
			return 1.15;
		} else {
			return 1.0;
		}
	}

	void DoFootTrample(Actor* giant, Actor* tiny, bool Right) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		ShrinkUntil(giant, tiny, 4.2, 0.22, false);

		std::string name = std::format("FootTrample_{}", tiny->formID);
		auto FrameA = Time::FramesElapsed();

		auto coordinates = AttachToUnderFoot(giant, tiny, Right); // get XYZ;
		
		SetBeingGrinded(tiny, true);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 4.0) {
				return true;
			}

			float zpos = coordinates.z;

			zpos = AttachToUnderFoot(giant, tiny, Right).z; // fix Z if it is wrong

			NiPoint3 attach = NiPoint3(coordinates.x, coordinates.y, zpos);

			AttachTo(giantref, tinyref, attach);
			if (!isTrampling(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
	}


	void DoFootGrind(Actor* giant, Actor* tiny, bool Right) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		ShrinkUntil(giant, tiny, 4.2, 0.16, false);
		
		std::string name = std::format("FootGrind_{}", tiny->formID);
		auto FrameA = Time::FramesElapsed();

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 4.0) {
				return true;
			}

			auto coordinates = AttachToUnderFoot(giant, tiny, Right);
				
			if (coordinates == NiPoint3(0,0,0)) {
				return true;
			}

			AttachTo(giantref, tinyref, coordinates);
			if (!IsFootGrinding(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
	}

	void DoFingerGrind(Actor* giant, Actor* tiny) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		ShrinkUntil(giant, tiny, 10.0, 0.18, false);
		
		std::string name = std::format("FingerGrind_{}_{}", giant->formID, tiny->formID);
		AnimationManager::StartAnim("Tiny_Finger_Impact_S", tiny);
		auto FrameA = Time::FramesElapsed();
		auto coordinates = AttachToObjectB_GetCoords(giant, tiny);
		if (coordinates == NiPoint3(0,0,0)) {
			return;
		}
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 3.0) {
				return true;
			}

			AttachTo(giantref, tinyref, coordinates);
			if (!IsFootGrinding(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
	}

	void FingerGrindCheck(Actor* giant, CrawlEvent kind, bool Right, float radius) {
		std::string_view name = GetImpactNode(kind);

		auto node = find_node(giant, name);
		if (!node) {
			return; // Make sure to return if node doesn't exist, no CTD in that case
		}

		if (!node) {
			return;
		}
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = Action_FingerGrind;
		bool SMT = HasSMT(giant);
		if (SMT) {
			SCALE_RATIO = 0.9;
		}

		NiPoint3 NodePosition = node->world.translate;

		float maxDistance = radius * giantScale;
		float CheckDistance = 220 * giantScale;
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, 0.0, 0.0), // The standard position
		};
		std::vector<NiPoint3> CrawlPoints = {};

		for (NiPoint3 point: points) {
			CrawlPoints.push_back(NodePosition);
		}
		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
			for (auto point: CrawlPoints) {
				DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance);
			}
		}

		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					for (auto point: CrawlPoints) {
						if ((actorLocation-giantLocation).Length() <= CheckDistance) {
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();
							if (model) {
								VisitNodes(model, [&nodeCollisions, &force, NodePosition, maxDistance](NiAVObject& a_obj) {
									float distance = (NodePosition - a_obj.world.translate).Length();
									if (distance < maxDistance) {
										nodeCollisions += 1;
										force = 1.0 - distance / maxDistance;
										return false;
									}
									return true;
								});
							}
							if (nodeCollisions > 0 && !otherActor->IsDead()) {
								SetBeingGrinded(otherActor, true);
								if (Right) {
									DoFingerGrind(giant, otherActor);
									AnimationManager::StartAnim("GrindRight", giant);
								} else {
									DoFingerGrind(giant, otherActor);
									AnimationManager::StartAnim("GrindLeft", giant);
								}
							}
						}
					}
				}
			}
		}
	}

	void FootGrindCheck_Left(Actor* actor, float radius, bool strong) {  // Check if we hit someone with stomp. Yes = Start foot grind. Left Foot.
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		float SCALE_RATIO = 3.0;


		if (HasSMT(actor)) {
			SCALE_RATIO = 0.8;
		}

		// Get world HH offset
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		auto leftFoot = find_node(actor, leftFootLookup);
		auto leftCalf = find_node(actor, leftCalfLookup);
		auto leftToe = find_node(actor, leftToeLookup);
		if (!leftFoot) {
			return;
		}
		if (!leftCalf) {
			return;
		}
		if (!leftToe) {
			return;
		}
		NiMatrix3 leftRotMat;
		{
			NiAVObject* foot = leftFoot;
			NiAVObject* calf = leftCalf;
			NiAVObject* toe = leftToe;
			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = forward.UnitCross(up);
			forward = up.UnitCross(right); // Reorthonalize

			leftRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 800, {0.0, 1.0, 0.0, 1.0});
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
											return false;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								float aveForce = std::clamp(force, 0.00f, 0.70f);
								ActorHandle giantHandle = actor->CreateRefHandle();
								ActorHandle tinyHandle = otherActor->CreateRefHandle();
								std::string taskname = std::format("GrindCheckL_{}_{}", actor->formID, otherActor->formID);
								TaskManager::RunOnce(taskname, [=](auto& update){
									if (!tinyHandle) {
										return;
									}
									if (!giantHandle) {
										return;
									}
									
									auto giant = giantHandle.get().get();
									auto tiny = tinyHandle.get().get();

									if (CanDoDamage(giant, tiny, false)) {
										if (aveForce >= 0.00 && !tiny->IsDead()) {
											SetBeingGrinded(tiny, true);
											if (!strong) {
												DoFootGrind(giant, tiny, false);
												AnimationManager::StartAnim("GrindLeft", giant);
											} else {
												AnimationManager::StartAnim("TrampleStartL", giant);
												DoFootTrample(giant, tiny, false);
											}
										}
									}
								});
							}
						}
					}
				}
			}
		}
	}

	void FootGrindCheck_Right(Actor* actor, float radius, bool strong) {  // Check if we hit someone with stomp. Yes = Start foot grind. Right Foot.
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		float SCALE_RATIO = 3.0;


		if (HasSMT(actor)) {
			SCALE_RATIO = 0.8;
		}
		
		// Get world HH offset
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		auto rightFoot = find_node(actor, rightFootLookup);
		auto rightCalf = find_node(actor, rightCalfLookup);
		auto rightToe = find_node(actor, rightToeLookup);

		if (!rightFoot) {
			return;
		}
		if (!rightCalf) {
			return;
		}
		if (!rightToe) {
			return;
		}
		NiMatrix3 rightRotMat;
		{
			NiAVObject* foot = rightFoot;
			NiAVObject* calf = rightCalf;
			NiAVObject* toe = rightToe;

			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = up.UnitCross(forward);
			forward = right.UnitCross(up); // Reorthonalize

			rightRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 800, {0.0, 1.0, 0.0, 1.0});
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor) * GetSizeFromBoundingBox(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;
											return false;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								float aveForce = std::clamp(force, 0.00f, 0.70f);
								ActorHandle giantHandle = actor->CreateRefHandle();
								ActorHandle tinyHandle = otherActor->CreateRefHandle();
								std::string taskname = std::format("GrindCheckR_{}_{}", actor->formID, otherActor->formID);
								TaskManager::RunOnce(taskname, [=](auto& update){
									if (!tinyHandle) {
										return;
									}
									if (!giantHandle) {
										return;
									}
									
									auto giant = giantHandle.get().get();
									auto tiny = tinyHandle.get().get();

									if (CanDoDamage(giant, tiny, false)) {
										if (aveForce >= 0.00 && !tiny->IsDead()) {
											SetBeingGrinded(tiny, true);
											if (!strong) {
												DoFootGrind(giant, tiny, true);
												AnimationManager::StartAnim("GrindRight", giant);
											} else {
												AnimationManager::StartAnim("TrampleStartR", giant);
												DoFootTrample(giant, tiny, true);
											}
										}
									}
								});
							}
						}
					}
				}
			}
		}
	}

	void DoDamageAtPoint_Cooldown(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float pushpower, DamageSource Cause) { // Apply crawl damage to each bone individually
		auto profiler = Profilers::Profile("Other: CrawlDamage");
		if (!node) {
			return;
		}
		if (!giant) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = 1.0;
		bool SMT = false;


		if (HasSMT(giant)) {
			giantScale += 2.40; // enough to push giants around, but not mammoths/dragons
			SMT = true; // set SMT to true
		}

		NiPoint3 NodePosition = node->world.translate;

		if (Cause == DamageSource::KickedLeft || Cause == DamageSource::KickedRight) {
			// Apply Down offset in that case
			float HH = HighHeelManager::GetHHOffset(giant).Length();
			NodePosition.z -= HH * 0.75;
		}

		float maxDistance = radius * giantScale;
		float CheckDistance = 220 * giantScale;

		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
			DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), maxDistance, 400.0);
		}

		NiPoint3 giantLocation = giant->GetPosition();

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				NiPoint3 actorLocation = otherActor->GetPosition();
				if ((actorLocation - giantLocation).Length() < CheckDistance) {
					tinyScale *= GetSizeFromBoundingBox(otherActor); // take Giant/Dragon scale into account

					int nodeCollisions = 0;
					float force = 0.0;

					auto model = otherActor->GetCurrent3D();

					if (model) {
						VisitNodes(model, [&nodeCollisions, &force, NodePosition, maxDistance](NiAVObject& a_obj) {
							float distance = (NodePosition - a_obj.world.translate).Length();
							if (distance < maxDistance) {
								nodeCollisions += 1;
								force = 1.0 - distance / maxDistance;
								return false;
							}
							return true;
						});
					}
					if (nodeCollisions > 0) {
						bool allow = IsActionOnCooldown(otherActor, CooldownSource::Damage_Hand);
						if (!allow) {
							float aveForce = std::clamp(force, 0.16f, 0.70f);
							float pushForce = std::clamp(force, 0.04f, 0.10f);
							float audio = 1.0;
							if (SMT) {
								pushForce *= 1.5;
								audio = 3.0;
							}
							if (otherActor->IsDead()) {
								tinyScale *= 0.6;
							}

							float difference = giantScale / tinyScale;
							float Threshold = GetStaggerThreshold(Cause);

							int Random = rand() % 100 + 1;
							int RagdollChance = (-32 + (32 / Threshold) * difference);
							bool roll = RagdollChance > Random;
							//log::info("Roll: {}, RandomChance {}, Threshold: {}", roll, RagdollChance, Random);
							//eventually it reaches 100% chance to ragdoll an actor (at ~x3.0 size difference)

							if (difference > 1.35 && (roll || otherActor->IsDead())) {
								PushTowards(giant, otherActor, node, pushForce * pushpower, true);
							} else if (difference > 0.88 * Threshold) {
								float push = std::clamp(0.25f * (difference - 0.25f), 0.25f, 1.0f);
								StaggerActor(giant, otherActor, push);
							}

							float Volume = std::clamp(difference*pushForce, 0.15f, 1.0f);

							auto node = find_node(giant, GetDeathNodeName(Cause));
							if (node) {
								Runtime::PlaySoundAtNode("SwingImpact", giant, Volume, 1.0, node); // play swing impact sound
							}

							ApplyActionCooldown(otherActor, CooldownSource::Damage_Hand);
							ApplyShakeAtPoint(giant, 3.0 * pushpower * audio, node->world.translate, 1.5, 0.0, 1.0);
							CollisionDamage::GetSingleton().DoSizeDamage(giant, otherActor, damage, bbmult, crushmult, random, Cause, true);
						}
					}
				}
			}
		}
	}

	void ApplyThighDamage(Actor* actor, bool right, bool CooldownCheck, float radius, float damage, float bbmult, float crush_threshold, int random, DamageSource Cause) {
		auto profiler = Profilers::Profile("CollisionDamageLeft: DoFootCollision_Left");
		auto& CollisionDamage = CollisionDamage::GetSingleton();
		if (!actor) {
			return;
		}
		
		auto& sizemanager = SizeManager::GetSingleton();
		float giantScale = get_visual_scale(actor);
		float perk = GetPerkBonus_Thighs(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		float damage_zones_applied = 0.0;
		float SCALE_RATIO = 1.75;

		if (HasSMT(actor)) {
			giantScale += 0.20;
			SCALE_RATIO = 0.90;
		}

		std::string_view leg = "NPC R Foot [Rft ]";
		std::string_view knee = "NPC R Calf [RClf]";
		std::string_view thigh = "NPC R Thigh [RThg]";

		if (!right) {
			leg = "NPC L Foot [Lft ]";
			knee = "NPC L Calf [LClf]";
			thigh = "NPC L Thigh [LThg]";
		}


		std::vector<NiPoint3> ThighPoints = GetThighCoordinates(actor, knee, leg, thigh);

		float speed = AnimationManager::GetBonusAnimationSpeed(actor);
		crush_threshold *= (1.10 - speed*0.10);

		float feet_damage = (Damage_ThighCrush_CrossLegs_FeetImpact * perk * speed);
		
		if (CooldownCheck) {
			CollisionDamage::GetSingleton().DoFootCollision(actor, feet_damage, radius, random, bbmult, crush_threshold, DamageSource::ThighCrushed, true, true);
			CollisionDamage::GetSingleton().DoFootCollision(actor, feet_damage, radius, random, bbmult, crush_threshold, DamageSource::ThighCrushed, false, true);
		}

		if (!ThighPoints.empty()) {
			for (const auto& point: ThighPoints) {
				float maxFootDistance = radius * giantScale;

				if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance);
				}
			
				NiPoint3 giantLocation = actor->GetPosition();
				for (auto otherActor: find_actors()) {
					if (otherActor != actor) {
						float tinyScale = get_visual_scale(otherActor);
						if (giantScale / tinyScale > SCALE_RATIO) {
							NiPoint3 actorLocation = otherActor->GetPosition();

							if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
								int nodeCollisions = 0;
								float force = 0.0;

								auto model = otherActor->GetCurrent3D();

								if (model) {
									for (auto point: ThighPoints) {
										VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
											float distance = (point - a_obj.world.translate).Length();
											if (distance < maxFootDistance) {
												nodeCollisions += 1;
												force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
												return false;
											}
											return true;
										});
									}
								}
								if (nodeCollisions > 0) {
									damage_zones_applied += 1.0;
									if (damage_zones_applied < 1.0) {
										damage_zones_applied = 1.0; // just to be safe
									}
									damage /= damage_zones_applied;
									if (CooldownCheck) {
										float pushForce = std::clamp(force, 0.04f, 0.10f);
										bool OnCooldown = IsActionOnCooldown(otherActor, CooldownSource::Damage_Thigh);
										if (!OnCooldown) {
											float pushCalc = 0.06 * pushForce * speed;
											Laugh_Chance(actor, otherActor, 1.35, "ThighCrush");
											float difference = giantScale / (tinyScale * GetSizeFromBoundingBox(otherActor));
											PushTowards(actor, otherActor, leg, pushCalc * difference, true);
											CollisionDamage.DoSizeDamage(actor, otherActor, damage * speed * perk, bbmult, crush_threshold, random, Cause, true);
											ApplyActionCooldown(otherActor, CooldownSource::Damage_Thigh);
										}
									} else {
										Utils_PushCheck(actor, otherActor, Get_Bone_Movement_Speed(actor, Cause)); // pass original un-altered force
										CollisionDamage.DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, true);
									}
								}
							}
						}
					}
				}
			}
		} 
	}

	void ApplyFingerDamage(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float Shrink, DamageSource Cause) { // Apply crawl damage to each bone individually
		auto profiler = Profilers::Profile("Other: FingerDamage");
		if (!node) {
			return;
		}
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = 1.25;
		if (HasSMT(giant)) {
			SCALE_RATIO = 0.8;
			giantScale *= 1.3;
		}
		NiPoint3 NodePosition = node->world.translate;

		float maxDistance = radius * giantScale;
		float CheckDistance = 220 * giantScale;
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, 0.0, 0.0), // The standard position
		};
		std::vector<NiPoint3> FingerPoints = {};

		for (NiPoint3 point: points) {
			FingerPoints.push_back(NodePosition);
		}
		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
			for (auto point: FingerPoints) {
				DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance, 400.0);
			}
		}

		Utils_UpdateHighHeelBlend(giant, false);
		NiPoint3 giantLocation = giant->GetPosition();
		
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					for (auto point: FingerPoints) {
						if ((actorLocation-giantLocation).Length() <= CheckDistance) {

							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								VisitNodes(model, [&nodeCollisions, &force, NodePosition, maxDistance](NiAVObject& a_obj) {
									float distance = (NodePosition - a_obj.world.translate).Length();
									if (distance < maxDistance) {
										nodeCollisions += 1;
										force = 1.0 - distance / maxDistance;
										return false;
									}
									return true;
								});
							}
							if (nodeCollisions > 0) {
								if (get_target_scale(otherActor) > 0.08 / GetSizeFromBoundingBox(otherActor)) {
									update_target_scale(otherActor, Shrink, SizeEffectType::kShrink);
								} else {
									set_target_scale(otherActor, 0.08 / GetSizeFromBoundingBox(otherActor));
								}
								Laugh_Chance(giant, otherActor, 1.0, "FingerGrind"); 

								Utils_PushCheck(giant, otherActor, 1.0);

								CollisionDamage::GetSingleton().DoSizeDamage(giant, otherActor, damage, bbmult, crushmult, random, Cause, true);
							}
						}
					}
				}
			}
		}
	}

	std::vector<NiPoint3> GetThighCoordinates(Actor* giant, std::string_view calf, std::string_view feet, std::string_view thigh) {
		NiAVObject* Knee = find_node(giant, calf);
		NiAVObject* Foot = find_node(giant, feet);
		NiAVObject* Thigh = find_node(giant, thigh);

		if (!Knee) {
			return std::vector<NiPoint3>{};
		}
		if (!Foot) {
			return std::vector<NiPoint3>{};
		}
		if (!Thigh) {
			return std::vector<NiPoint3>{};
		}

		NiPoint3 Knee_Point = Knee->world.translate;
		NiPoint3 Foot_Point = Foot->world.translate;
		NiPoint3 Thigh_Point = Thigh->world.translate;

		NiPoint3 Knee_Pos_Middle = (Knee_Point + Foot_Point) / 2.0; 				// middle  |-----|-----|
		NiPoint3 Knee_Pos_Up = (Knee_Point + Knee_Pos_Middle) / 2.0;				//         |--|--|-----|
		NiPoint3 Knee_Pos_Down = (Knee_Pos_Middle + Foot_Point) / 2.0; 				//         |-----|--|--|

		NiPoint3 Thigh_Pos_Middle = (Thigh_Point + Knee_Point) / 2.0;               // middle  |-----|-----|
		NiPoint3 Thigh_Pos_Up = (Thigh_Pos_Middle + Thigh_Point) / 2.0;            	//         |--|--|-----|
		NiPoint3 Thigh_Pos_Down = (Thigh_Pos_Middle + Knee_Point) / 2.0;        	//         |-----|--|--|

		NiPoint3 Knee_Thigh_Middle = (Thigh_Pos_Down + Knee_Pos_Up) / 2.0;          // middle between two

		std::vector<NiPoint3> coordinates = { 	
			Knee_Pos_Middle,
			Knee_Pos_Up,
			Knee_Pos_Down,
			Thigh_Pos_Middle,
			Thigh_Pos_Up,
			Thigh_Pos_Down,
			Knee_Thigh_Middle,
		};

		return coordinates;
	}

	NiPoint3 GetHeartPosition(Actor* giant, Actor* tiny) { // It is used to spawn Heart Particles during healing hugs

		NiPoint3 TargetA = NiPoint3();
		NiPoint3 TargetB = NiPoint3();
		std::vector<std::string_view> bone_names = {
			"L Breast03",
			"R Breast03"
		};
		std::uint32_t bone_count = bone_names.size();
		for (auto bone_name_A: bone_names) {
			auto bone = find_node(giant, bone_name_A);
			if (!bone) {
				Notify("Error: Breast Nodes could not be found.");
				Notify("Actor without nodes: {}", giant->GetDisplayFullName());
				Notify("Suggestion: install XP32 skeleton.");
				return NiPoint3();
			}
			TargetA += (bone->world.translate) * (1.0/bone_count);
		}
		for (auto bone_name_B: bone_names) {
			auto bone = find_node(tiny, bone_name_B);
			if (!bone) {
				Notify("Error: Breast Nodes could not be found.");
				Notify("Actor without nodes: {}", tiny->GetDisplayFullName());
				Notify("Suggestion: install XP32 skeleton.");
				return NiPoint3();
			}
			TargetB += (bone->world.translate) * (1.0/bone_count);
		}

		auto targetPoint = (TargetA + TargetB) / 2;
		targetPoint.z += 45.0 * get_visual_scale(giant);
		return targetPoint;
	}

	void AbsorbShout_BuffCaster(Actor* giantref, Actor* tinyref) {
		static Timer MoanTimer = Timer(10.0);
		auto random = rand() % 8;
		if (random <= 4) {
			if (MoanTimer.ShouldRunFrame()) {
				ApplyShakeAtNode(giantref, 6.0, "NPC COM [COM ]", 124.0);
				ModSizeExperience(giantref, 0.14);
				PlayMoanSound(giantref, 1.0);

				Grow(giantref, 0, 0.016 * (1 + random));

				Runtime::CastSpell(giantref, giantref, "GtsVoreFearSpell");

				SpawnCustomParticle(giantref, ParticleType::Blue, NiPoint3(), "NPC COM [COM ]", get_visual_scale(giantref));
				Task_FacialEmotionTask_Moan(giantref, 2.0, "Absorb");
			}	
		}
	}

	void Task_TrackSizeTask(Actor* giant, Actor* tiny, std::string_view naming) { 
		// A fail-safe task. The goal of it is to kill actor
		// if half-life puts actor below shrink to nothing threshold, so we won't have < x0.01 actors
		ActorHandle giantHandle = giant->CreateRefHandle();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		
		float task_duration = 3.0;
		std::string name = std::format("{}_STN_Check_{}_{}", naming, giant->formID, tiny->formID);

		TaskManager::RunFor(name, task_duration, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}
			if (!tinyHandle) {
				return false;
			}
			auto giantref = giantHandle.get().get();
			auto tinyref = tinyHandle.get().get();

			float size = get_visual_scale(tinyref);
			if (ShrinkToNothing(giantref, tinyref)) {
				if (naming == "Absorb") {
					AbsorbShout_BuffCaster(giantref, tinyref);
				}
				return false;
			}

			return true;
		});
	}

	void Task_FacialEmotionTask_Moan(Actor* giant, float duration, std::string_view naming) {
		ActorHandle giantHandle = giant->CreateRefHandle();

		float start = Time::WorldTimeElapsed();
		std::string name = std::format("{}_Facial_{}", naming, giant->formID);

		AdjustFacialExpression(giant, 0, 1.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 1.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 0, 0.75, "phenome"); // open mouth

		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}
			float finish = Time::WorldTimeElapsed();
			auto giantref = giantHandle.get().get();
			float timepassed = finish - start;
			if (timepassed >= duration) {
				AdjustFacialExpression(giantref, 0, 0.0, "modifier"); // blink L
				AdjustFacialExpression(giantref, 1, 0.0, "modifier"); // blink R
				AdjustFacialExpression(giantref, 0, 0.0, "phenome"); // close mouth
				return false;
			}
			return true;
		});
	}

	void Task_FacialEmotionTask_Smile(Actor* giant, float duration, std::string_view naming) {
		ActorHandle giantHandle = giant->CreateRefHandle();

		float start = Time::WorldTimeElapsed();
		std::string name = std::format("{}_Facial_{}", naming, giant->formID);

		AdjustFacialExpression(giant, 0, 0.1, "phenome"); // Start opening mouth

		AdjustFacialExpression(giant, 0, 0.40, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.40, "modifier"); // blink R

		float random = (rand()% 25) * 0.01;
		log::info("Smile Random: {}", random);
		float smile = 0.25 + (random); // up to +0.50 to open mouth

		AdjustFacialExpression(giant, 3, random, "phenome"); // Slightly open mouth
		AdjustFacialExpression(giant, 7, 1.0, "phenome"); // Close mouth stronger to counter opened mouth from smiling
		AdjustFacialExpression(giant, 5, 0.5, "phenome"); // Actual smile but leads to opening mouth 

		// Emotion guide:
		// https://steamcommunity.com/sharedfiles/filedetails/?id=187155077

		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}
			float finish = Time::WorldTimeElapsed();
			auto giantref = giantHandle.get().get();
			float timepassed = finish - start;
			if (timepassed >= duration) {
				AdjustFacialExpression(giant, 0, 0.0, "phenome"); // Start opening mouth

				AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
				AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R

				AdjustFacialExpression(giant, 3, 0.0, "phenome"); // Smile a bit (Mouth)
				return false;
			}
			return true;
		});
	}

	void Laugh_Chance(Actor* giant, Actor* otherActor, float multiply, std::string_view name) {
		bool Blocked = IsActionOnCooldown(giant, CooldownSource::Emotion_Laugh);
		if (!Blocked) {
			int rng = rand() % 2 + 1;
			if (rng <= 1.0) {
				float duration = 1.5 + ((rand() % 100) * 0.01);
				duration *= multiply;

				ApplyActionCooldown(giant, CooldownSource::Emotion_Laugh);
				
				if (!otherActor->IsDead()) {
					PlayLaughSound(giant, 1.0, 1);
					Task_FacialEmotionTask_Smile(giant, duration, name);
					
				}
			}
		}
	}

	void Laugh_Chance(Actor* giant, float multiply, std::string_view name) {
		bool Blocked = IsActionOnCooldown(giant, CooldownSource::Emotion_Laugh);
		if (!Blocked) {
			int rng = rand() % 2 + 1;
			if (rng <= 1.0) {
				float duration = 1.5 + ((rand() % 100) * 0.01);
				duration *= multiply;

				PlayLaughSound(giant, 1.0, 1);
				Task_FacialEmotionTask_Smile(giant, duration, name);
				ApplyActionCooldown(giant, CooldownSource::Emotion_Laugh);
			}
		}
	}

	float GetHugStealRate(Actor* actor) {
		float steal = 0.18;
		if (Runtime::HasPerkTeam(actor, "HugCrush_ToughGrip")) {
			steal += 0.072;
		}
		if (Runtime::HasPerkTeam(actor, "HugCrush")) {
			steal *= 1.35;
		}
		return steal;
	}

	float GetHugShrinkThreshold(Actor* actor) {
		float threshold = 2.5;
		float bonus = 1.0;
		if (Runtime::HasPerk(actor, "HugCrush")) {
			bonus += 0.25;
		}
		if (Runtime::HasPerk(actor, "HugCrush_Greed")) {
			bonus += 0.35;
		}
		if (HasGrowthSpurt(actor)) {
			bonus *= 2.0;
		}
		return threshold * bonus;
	}

	float GetHugCrushThreshold(Actor* actor) {
		float hp = 0.20;
		if (Runtime::HasPerkTeam(actor, "HugCrush_MightyCuddles")) {
			hp += 0.10; // 0.30
		}
		if (Runtime::HasPerkTeam(actor, "HugCrush_HugsOfDeath")) {
			hp += 0.20; // 0.50
		}
		return hp;
	}
}
