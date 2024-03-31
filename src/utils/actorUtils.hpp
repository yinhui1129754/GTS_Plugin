#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	[[nodiscard]] RE::NiPoint3 RotateAngleAxis(const RE::NiPoint3& vec, const float angle, const RE::NiPoint3& axis);

	Actor* GetActorPtr(Actor* actor);

	Actor* GetActorPtr(Actor& actor);

	Actor* GetActorPtr(ActorHandle& actor);

	Actor* GetActorPtr(const ActorHandle& actor);

	Actor* GetActorPtr(FormID formId);

	Actor* GetCharContActor(bhkCharacterController* charCont);

	float GetLaunchPower(Actor* giant, float sizeRatio);

	void StartResetTask(Actor* tiny);
	void PlayMoanSound(Actor* actor, float volume);
	void PlayLaughSound(Actor* actor, float volume, int type);

	// GTS State Bools
	bool BehaviorGraph_DisableHH(Actor* actor);
	bool IsEquipBusy(Actor* actor);
	
	bool IsRagdolled(Actor* actor);
	bool IsChangingSize(Actor* actor);

	bool IsFootGrinding(Actor* actor);
	bool isTrampling(Actor* actor);
	bool IsProning(Actor* actor);
	bool IsCrawling(Actor* actor);
	bool IsInBalanceMode();
	bool IsHugCrushing(Actor* actor);
	bool IsHugHealing(Actor* actor);
	bool IsHuggingFriendly(Actor* actor);
	bool IsTransitioning(Actor* actor);
	bool IsJumping(Actor* actor);
	bool IsBeingHeld(Actor* giant, Actor* tiny);
	bool IsBetweenBreasts(Actor* actor);
	bool IsTransferingTiny(Actor* actor);
	bool IsUsingThighAnimations(Actor* actor);
	bool IsSynced(Actor* actor);
	bool CanDoPaired(Actor* actor);
	bool IsThighCrushing(Actor* actor);
	bool IsThighSandwiching(Actor* actor);
	bool IsStomping(Actor* actor);
	bool IsBeingEaten(Actor* tiny);
	bool IsGtsBusy(Actor* actor);
	bool CanDoCombo(Actor* actor);
	bool IsCameraEnabled(Actor* actor);
	bool IsCrawlVoring(Actor* actor);
	bool IsButtCrushing(Actor* actor);
	bool ButtCrush_IsAbleToGrow(Actor* actor, float limit);
	bool IsBeingGrinded(Actor* actor);
	bool IsHugging(Actor* actor);
	bool IsBeingHugged(Actor* actor); 
	bool CanDoButtCrush(Actor* actor, bool apply_cooldown);
	bool GetCameraOverride(Actor* actor);
	// GTS State Bools End

	// Gts Bools
	bool IsGrowthSpurtActive(Actor* actor);
	bool HasGrowthSpurt(Actor* actor);
	bool InBleedout(Actor* actor);
	bool AllowStagger(Actor* giant, Actor* tiny);
	bool IsMechanical(Actor* actor);
	bool IsHuman(Actor* actor);
	bool IsBlacklisted(Actor* actor);

	void Potion_SetMightBonus(Actor* giant, float value, bool add);
	float Potion_GetMightBonus(Actor* giant);

	void Potion_SetShrinkResistance(Actor* giant, float value);
	float Potion_GetShrinkResistance(Actor* giant);

	void Potion_SetUnderGrowth(Actor* actor, bool set);
	bool Potion_IsUnderGrowthPotion(Actor* actor);
	
	bool IsInsect(Actor* actor, bool performcheck);
	bool IsFemale(Actor* actor);
	bool IsDragon(Actor* actor);
	bool IsGiant(Actor* actor);
	bool IsMammoth(Actor* actor);
	bool IsLiving(Actor* actor);
	bool IsUndead(Actor* actor, bool PerformCheck);
	bool WasReanimated(Actor* actor);
	bool IsHeadtracking(Actor* giant);
	bool IsHostile(Actor* giant, Actor* tiny);
	bool CanPerformAnimationOn(Actor* giant, Actor* tiny);
	bool IsEssential(Actor* actor);
	bool AnimationsInstalled(Actor* giant);
	bool IsInGodMode(Actor* giant);
	bool IsFreeCameraEnabled();
	bool SizeRaycastEnabled();
	bool IsDebugEnabled();
	bool CanDoDamage(Actor* giant, Actor* tiny, bool HoldCheck);

	void ControlAnother(Actor* target, bool reset);
	Actor* GetPlayerOrControlled();



	// Gts Bools end

	// GTS Actor Functions
	float GetDamageSetting();
	float GetFallModifier(Actor* giant);

	

	float GetHPThreshold(Actor* actor);

	float Ench_Aspect_GetPower(Actor* giant);
	float Ench_Hunger_GetPower(Actor* giant);

	float GetDamageResistance(Actor* actor);
	float GetDamageMultiplier(Actor* actor);
	float Damage_CalculateSizeDamage(Actor* giant, Actor* tiny);

	float GetSizeDifference(Actor* giant, Actor* tiny, SizeType Type, bool Check_SMT, bool HH);
	float GetActorWeight(Actor* giant, bool metric);
	float GetActorHeight(Actor* giant, bool metric);
	float GetSizeFromBoundingBox(Actor* tiny);
	float GetRoomStateScale(Actor* giant);
	float GetProneAdjustment();

	void update_target_scale(Actor* giant, float amt, SizeEffectType type);
	float get_update_target_scale(Actor* giant, float amt, SizeEffectType type);


	void SpawnActionIcon(Actor* giant);
	// End

	// GTS State Controllers
	void SetBeingHeld(Actor* tiny, bool decide);
	void SetProneState(Actor* giant, bool enable);
	void SetBetweenBreasts(Actor* actor, bool decide);
	void SetBeingEaten(Actor* tiny, bool decide);
	void SetBeingGrinded(Actor* tiny, bool decide);
	void SetCameraOverride(Actor* actor, bool decide);
	void SetReanimatedState(Actor* actor);
	void ShutUp(Actor* actor);

	// GTS State Controllers end
	void PlayAnimation(Actor* actor, std::string_view animName);

	void Disintegrate(Actor* actor, bool script);
	void UnDisintegrate(Actor* actor);

	void SetRestrained(Actor* actor);
	void SetUnRestrained(Actor* actor);

	void SetDontMove(Actor* actor);
	void SetMove(Actor* actor);

	void ForceRagdoll(Actor* actor, bool forceOn);

	std::vector<hkpRigidBody*> GetActorRBs(Actor* actor);
	void PushActorAway(Actor* source, Actor* receiver, float afKnockbackForce);
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius);
	void ApplyManualHavokImpulse(Actor* target, float afX, float afY, float afZ, float Multiplier);
	void ApplyHavokImpulse(TESObjectREFR* target, float afX, float afY, float afZ, float afMagnitude);

	void CompleteDragonQuest(Actor* tiny, bool vore, bool dead);

	float get_distance_to_actor(Actor* receiver, Actor* target);
	float GetHighHeelsBonusDamage(Actor* actor);

	void ApplyShake(Actor* caster, float modifier);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node, float radius);
	void ApplyShakeAtPoint(Actor* caster, float modifier, const NiPoint3& coords, float radius);
	void EnableFreeCamera();

	bool DisallowSizeDamage(Actor* giant, Actor* tiny);
	bool AllowDevourment();
	bool AllowCameraTracking();
	bool LessGore();

	bool IsTeammate(Actor* actor);
	bool EffectsForEveryone(Actor* giant);
	
	void ResetCameraTracking();
	void CallDevourment(Actor* giant, Actor* tiny);
	void CallGainWeight(Actor* giant, float value);
	void CallVampire();
	void CallHelpMessage();
	void AddCalamityPerk();
	void AddPerkPoints(float level);

	void AddStolenAttributes(Actor* giant, float value);
	void AddStolenAttributesTowards(Actor* giant, ActorValue type, float value);
	float GetStolenAttributes_Values(Actor* giant, ActorValue type);
	float GetStolenAttributes(Actor* giant);
	void DistributeStolenAttributes(Actor* giant, float value);

	float GetRandomBoost();
	
	float GetButtCrushCost(Actor* actor);
	float Perk_GetCostReduction(Actor* giant);
	float GetAnimationSlowdown(Actor* giant);

	void DoFootstepSound(Actor* giant, float modifier, FootEvent kind, std::string_view node);
	void DoDustExplosion(Actor* giant, float modifier, FootEvent kind, std::string_view node);
	void SpawnParticle(Actor* actor, float lifetime, const char* modelName, const NiMatrix3& rotation, const NiPoint3& position, float scale, std::uint32_t flags, NiAVObject* target);
	void SpawnDustParticle(Actor* giant, Actor* tiny, std::string_view node, float size);
	void SpawnDustExplosion(Actor* giant, Actor* tiny, std::string_view node, float size);

	bool CanPush(Actor* tiny);
	void SetCanBePushed(Actor* tiny, bool prevent);

	void Utils_PushCheck(Actor* giant, Actor* tiny, float force);
	void StaggerOr(Actor* giant, Actor* tiny, float afX, float afY, float afZ, float afMagnitude);
	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage, FootEvent kind, float crushmult, DamageSource Cause);

	void PushTowards(Actor* giantref, Actor* tinyref, std::string_view bone, float power, bool sizecheck);
	void PushTowards_Task(ActorHandle giantHandle, ActorHandle tinyHandle, const NiPoint3& startCoords, const NiPoint3& endCoords, std::string_view TaskName, float power, bool sizecheck);
	void PushTowards(Actor* giantref, Actor* tinyref, NiAVObject* bone, float power, bool sizecheck);
	void PushForward(Actor* giantref, Actor* tinyref, float power);
	void TinyCalamityExplosion(Actor* giant, float radius);
	void ShrinkOutburst_Shrink(Actor* giant, Actor* tiny, float shrink, float gigantism);
	void ShrinkOutburstExplosion(Actor* giant, bool WasHit);

	void Utils_ProtectTinies(bool Balance);
	void LaunchImmunityTask(Actor* giant, bool Balance);

	bool HasSMT(Actor* giant);
	void TiredSound(Actor* player, std::string_view message);

	hkaRagdollInstance* GetRagdoll(Actor* actor);

	void ManageRagdoll(Actor* tinyref, float deltaLength, NiPoint3 deltaLocation, NiPoint3 targetLocation);
	void ChanceToScare(Actor* giant, Actor* tiny);
	void StaggerActor(Actor* receiver, float power);
	void StaggerActor(Actor* giant, Actor* tiny, float power);
	void StaggerActor_Around(Actor* giant, const float radius, bool launch);
	

	float GetMovementModifier(Actor* giant);
	float GetGtsSkillLevel();
	float GetXpBonus();

	void AddSMTDuration(Actor* actor, float duration);
	void AddSMTPenalty(Actor* actor, float penalty);

	void PrintDeathSource(Actor* giant, Actor* tiny, DamageSource cause);
	void PrintSuffocate(Actor* pred, Actor* prey);
	void ShrinkUntil(Actor* giant, Actor* tiny, float expected, float halflife, bool animation);
	void DisableCollisions(Actor* actor, TESObjectREFR* otherActor);
	void EnableCollisions(Actor* actor);

	void SpringGrow(Actor* actor, float amt, float halfLife, std::string_view naming);
	void SpringGrow_Free(Actor* actor, float amt, float halfLife, std::string_view naming);
	void SpringShrink(Actor* actor, float amt, float halfLife, std::string_view naming);

	void ResetGrab(Actor* giant);
	void FixAnimationsAndCamera();

	bool CanPerformAnimation(Actor* giant, float type);
	void AdvanceQuestProgression(Actor* giant, float stage, float value);
	void AdvanceQuestProgression(Actor* giant, Actor* tiny, float stage, float value, bool vore);
	void SpawnProgressionParticle(Actor* tiny, bool vore);
	void ResetQuest();
	float GetQuestProgression(float stage);
	void InflictSizeDamage(Actor* attacker, Actor* receiver, float value);

	float Sound_GetFallOff(NiAVObject* source, float mult);

	// RE Fun:
	void SetCriticalStage(Actor* actor, int stage);
	void Attacked(Actor* victim, Actor* agressor);
	void StartCombat(Actor* victim, Actor* agressor);
  	void ApplyDamage(Actor* giant, Actor* tiny, float damage);
	void SetObjectRotation_X(TESObjectREFR* ref, float X);
	void StaggerActor_Directional(Actor* giant, float power, Actor* tiny);
	void SetLinearImpulse(bhkRigidBody* body, const hkVector4& a_impulse);
	void SetAngularImpulse(bhkRigidBody* body, const hkVector4& a_impulse);
	void SetLinearVelocity(bhkRigidBody* body, const hkVector4& a_newVel);

	std::int16_t GetItemCount(InventoryChanges* changes, RE::TESBoundObject* a_obj);
	int GetCombatState(Actor* actor);
	bool IsMoving(Actor* giant);
}
