#pragma once
#include <fmt/core.h>
#include <spdlog/fmt/ostr.h>

using namespace RE;

template <> struct fmt::formatter<BSFixedString> : formatter<string_view> {
	auto format(BSFixedString v, format_context& ctx) const {
		string_view name = "<empty>";
		if (v.c_str()) {
			name = v.c_str();
		}
		return formatter<string_view>::format(name, ctx);
	}
};


template <> struct fmt::formatter<ACTOR_VALUE_MODIFIER>: formatter<string_view> {
	auto format(ACTOR_VALUE_MODIFIER v, format_context& ctx) const {
		string_view name = "unknown";
		switch (v) {
			case ACTOR_VALUE_MODIFIER::kPermanent: name = "kPermanent"; break;
			case ACTOR_VALUE_MODIFIER::kTemporary: name = "kTemporary"; break;
			case ACTOR_VALUE_MODIFIER::kDamage: name = "kDamage"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};


template <> struct fmt::formatter<COL_LAYER>: formatter<string_view> {
	auto format(COL_LAYER v, format_context& ctx) const {
		string_view name = "unknown";
		switch (v) {
			case COL_LAYER::kUnidentified: name = "kUnidentified"; break;
			case COL_LAYER::kStatic: name = "kStatic"; break;
			case COL_LAYER::kAnimStatic: name = "kAnimStatic"; break;
			case COL_LAYER::kTransparent: name = "kTransparent"; break;
			case COL_LAYER::kClutter: name = "kClutter"; break;
			case COL_LAYER::kWeapon: name = "kWeapon"; break;
			case COL_LAYER::kProjectile: name = "kProjectile"; break;
			case COL_LAYER::kSpell: name = "kSpell"; break;
			case COL_LAYER::kBiped: name = "kBiped"; break;
			case COL_LAYER::kTrees: name = "kTrees"; break;
			case COL_LAYER::kProps: name = "kProps"; break;
			case COL_LAYER::kWater: name = "kWater"; break;
			case COL_LAYER::kTrigger: name = "kTrigger"; break;
			case COL_LAYER::kTerrain: name = "kTerrain"; break;
			case COL_LAYER::kTrap: name = "kTrap"; break;
			case COL_LAYER::kNonCollidable: name = "kNonCollidable"; break;
			case COL_LAYER::kCloudTrap: name = "kCloudTrap"; break;
			case COL_LAYER::kGround: name = "kGround"; break;
			case COL_LAYER::kPortal: name = "kPortal"; break;
			case COL_LAYER::kDebrisSmall: name = "kDebrisSmall"; break;
			case COL_LAYER::kDebrisLarge: name = "kDebrisLarge"; break;
			case COL_LAYER::kAcousticSpace: name = "kAcousticSpace"; break;
			case COL_LAYER::kActorZone: name = "kActorZone"; break;
			case COL_LAYER::kProjectileZone: name = "kProjectileZone"; break;
			case COL_LAYER::kGasTrap: name = "kGasTrap"; break;
			case COL_LAYER::kShellCasting: name = "kShellCasting"; break;
			case COL_LAYER::kTransparentWall: name = "kTransparentWall"; break;
			case COL_LAYER::kInvisibleWall: name = "kInvisibleWall"; break;
			case COL_LAYER::kTransparentSmallAnim: name = "kTransparentSmallAnim"; break;
			case COL_LAYER::kClutterLarge: name = "kClutterLarge"; break;
			case COL_LAYER::kCharController: name = "kCharController"; break;
			case COL_LAYER::kStairHelper: name = "kStairHelper"; break;
			case COL_LAYER::kDeadBip: name = "kDeadBip"; break;
			case COL_LAYER::kBipedNoCC: name = "kBipedNoCC"; break;
			case COL_LAYER::kAvoidBox: name = "kAvoidBox"; break;
			case COL_LAYER::kCollisionBox: name = "kCollisionBox"; break;
			case COL_LAYER::kCameraSphere: name = "kCameraSphere"; break;
			case COL_LAYER::kDoorDetection: name = "kDoorDetection"; break;
			case COL_LAYER::kConeProjectile: name = "kConeProjectile"; break;
			case COL_LAYER::kCamera: name = "kCamera"; break;
			case COL_LAYER::kItemPicker: name = "kItemPicker"; break;
			case COL_LAYER::kLOS: name = "kLOS"; break;
			case COL_LAYER::kPathingPick: name = "kPathingPick"; break;
			case COL_LAYER::kUnused0: name = "kUnused0"; break;
			case COL_LAYER::kUnused1: name = "kUnused1"; break;
			case COL_LAYER::kSpellExplosion: name = "kSpellExplosion"; break;
			case COL_LAYER::kDroppingPick: name = "kDroppingPick"; break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};

template <> struct fmt::formatter<DEFAULT_OBJECT>: formatter<string_view> {
	auto format(DEFAULT_OBJECT v, format_context& ctx) const {
		string_view name = "unknown";
		switch (v) {
			case DEFAULT_OBJECT::kWerewolfSpell: name = "kWerewolfSpell"; break;
			case DEFAULT_OBJECT::kSittingAngleLimit: name = "kSittingAngleLimit"; break;
			case DEFAULT_OBJECT::kAllowPlayerShout: name = "kAllowPlayerShout"; break;
			case DEFAULT_OBJECT::kGold: name = "kGold"; break;
			case DEFAULT_OBJECT::kLockpick: name = "kLockpick"; break;
			case DEFAULT_OBJECT::kSkeletonKey: name = "kSkeletonKey"; break;
			case DEFAULT_OBJECT::kPlayerFaction: name = "kPlayerFaction"; break;
			case DEFAULT_OBJECT::kGuardFaction: name = "kGuardFaction"; break;
			case DEFAULT_OBJECT::kDefaultMusic: name = "kDefaultMusic"; break;
			case DEFAULT_OBJECT::kBattleMusic: name = "kBattleMusic"; break;
			case DEFAULT_OBJECT::kDeathMusic: name = "kDeathMusic"; break;
			case DEFAULT_OBJECT::kSuccessMusic: name = "kSuccessMusic"; break;
			case DEFAULT_OBJECT::kLevelUpMusic: name = "kLevelUpMusic"; break;
			case DEFAULT_OBJECT::kDungeonClearedMusic: name = "kDungeonClearedMusic"; break;
			case DEFAULT_OBJECT::kPlayerVoiceMale: name = "kPlayerVoiceMale"; break;
			case DEFAULT_OBJECT::kPlayerVoiceMaleChild: name = "kPlayerVoiceMaleChild"; break;
			case DEFAULT_OBJECT::kPlayerVoiceFemale: name = "kPlayerVoiceFemale"; break;
			case DEFAULT_OBJECT::kPlayerVoiceFemaleChild: name = "kPlayerVoiceFemaleChild"; break;
			case DEFAULT_OBJECT::kEatPackageDefaultFood: name = "kEatPackageDefaultFood"; break;
			case DEFAULT_OBJECT::kLeftHandEquip: name = "kLeftHandEquip"; break;
			case DEFAULT_OBJECT::kRightHandEquip: name = "kRightHandEquip"; break;
			case DEFAULT_OBJECT::kEitherHandEquip: name = "kEitherHandEquip"; break;
			case DEFAULT_OBJECT::kVoiceEquip: name = "kVoiceEquip"; break;
			case DEFAULT_OBJECT::kPotionEquip: name = "kPotionEquip"; break;
			case DEFAULT_OBJECT::kEveryActorAbility: name = "kEveryActorAbility"; break;
			case DEFAULT_OBJECT::kCommandedActorAbility: name = "kCommandedActorAbility"; break;
			case DEFAULT_OBJECT::kDrugWearsOffImageSpace: name = "kDrugWearsOffImageSpace"; break;
			case DEFAULT_OBJECT::kFootstepSet: name = "kFootstepSet"; break;
			case DEFAULT_OBJECT::kLandscapeMaterial: name = "kLandscapeMaterial"; break;
			case DEFAULT_OBJECT::kDragonLandZoneMarker: name = "kDragonLandZoneMarker"; break;
			case DEFAULT_OBJECT::kDragonCrashZoneMarker: name = "kDragonCrashZoneMarker"; break;
			case DEFAULT_OBJECT::kCombatStyle: name = "kCombatStyle"; break;
			case DEFAULT_OBJECT::kDefaultPackList: name = "kDefaultPackList"; break;
			case DEFAULT_OBJECT::kWaitForDialoguePackage: name = "kWaitForDialoguePackage"; break;
			case DEFAULT_OBJECT::kLocRefTypeBoss: name = "kLocRefTypeBoss"; break;
			case DEFAULT_OBJECT::kVirtualLocation: name = "kVirtualLocation"; break;
			case DEFAULT_OBJECT::kPersistAllLocation: name = "kPersistAllLocation"; break;
			case DEFAULT_OBJECT::kInventoryPlayer: name = "kInventoryPlayer"; break;
			case DEFAULT_OBJECT::kPathingTestNPC: name = "kPathingTestNPC"; break;
			case DEFAULT_OBJECT::kFavorCostSmall: name = "kFavorCostSmall"; break;
			case DEFAULT_OBJECT::kFavorCostMedium: name = "kFavorCostMedium"; break;
			case DEFAULT_OBJECT::kFavorCostLarge: name = "kFavorCostLarge"; break;
			case DEFAULT_OBJECT::kFavorGiftsPerDay: name = "kFavorGiftsPerDay"; break;
			case DEFAULT_OBJECT::kActionSwimStateChange: name = "kActionSwimStateChange"; break;
			case DEFAULT_OBJECT::kActionLook: name = "kActionLook"; break;
			case DEFAULT_OBJECT::kActionLeftAttack: name = "kActionLeftAttack"; break;
			case DEFAULT_OBJECT::kActionLeftReady: name = "kActionLeftReady"; break;
			case DEFAULT_OBJECT::kActionLeftRelease: name = "kActionLeftRelease"; break;
			case DEFAULT_OBJECT::kActionLeftInterrupt: name = "kActionLeftInterrupt"; break;
			case DEFAULT_OBJECT::kActionRightAttack: name = "kActionRightAttack"; break;
			case DEFAULT_OBJECT::kActionRightReady: name = "kActionRightReady"; break;
			case DEFAULT_OBJECT::kActionRightRelease: name = "kActionRightRelease"; break;
			case DEFAULT_OBJECT::kActionRightInterrupt: name = "kActionRightInterrupt"; break;
			case DEFAULT_OBJECT::kActionDualAttack: name = "kActionDualAttack"; break;
			case DEFAULT_OBJECT::kActionDualRelease: name = "kActionDualRelease"; break;
			case DEFAULT_OBJECT::kActionActivate: name = "kActionActivate"; break;
			case DEFAULT_OBJECT::kActionJump: name = "kActionJump"; break;
			case DEFAULT_OBJECT::kActionFall: name = "kActionFall"; break;
			case DEFAULT_OBJECT::kActionLand: name = "kActionLand"; break;
			case DEFAULT_OBJECT::kActionSneak: name = "kActionSneak"; break;
			case DEFAULT_OBJECT::kActionVoice: name = "kActionVoice"; break;
			case DEFAULT_OBJECT::kActionVoiceReady: name = "kActionVoiceReady"; break;
			case DEFAULT_OBJECT::kActionVoiceRelease: name = "kActionVoiceRelease"; break;
			case DEFAULT_OBJECT::kActionVoiceInterrupt: name = "kActionVoiceInterrupt"; break;
			case DEFAULT_OBJECT::kActionIdle: name = "kActionIdle"; break;
			case DEFAULT_OBJECT::kActionSprintStart: name = "kActionSprintStart"; break;
			case DEFAULT_OBJECT::kActionSprintStop: name = "kActionSprintStop"; break;
			case DEFAULT_OBJECT::kActionDraw: name = "kActionDraw"; break;
			case DEFAULT_OBJECT::kActionSheath: name = "kActionSheath"; break;
			case DEFAULT_OBJECT::kActionLeftPowerAttack: name = "kActionLeftPowerAttack"; break;
			case DEFAULT_OBJECT::kActionRightPowerAttack: name = "kActionRightPowerAttack"; break;
			case DEFAULT_OBJECT::kActionDualPowerAttack: name = "kActionDualPowerAttack"; break;
			case DEFAULT_OBJECT::kActionStaggerStart: name = "kActionStaggerStart"; break;
			case DEFAULT_OBJECT::kActionBlockHit: name = "kActionBlockHit"; break;
			case DEFAULT_OBJECT::kActionBlockAnticipate: name = "kActionBlockAnticipate"; break;
			case DEFAULT_OBJECT::kActionRecoil: name = "kActionRecoil"; break;
			case DEFAULT_OBJECT::kActionLargeRecoil: name = "kActionLargeRecoil"; break;
			case DEFAULT_OBJECT::kActionBleedoutStart: name = "kActionBleedoutStart"; break;
			case DEFAULT_OBJECT::kActionBleedoutStop: name = "kActionBleedoutStop"; break;
			case DEFAULT_OBJECT::kActionIdleStop: name = "kActionIdleStop"; break;
			case DEFAULT_OBJECT::kActionWardHit: name = "kActionWardHit"; break;
			case DEFAULT_OBJECT::kActionForceEquip: name = "kActionForceEquip"; break;
			case DEFAULT_OBJECT::kActionShieldChange: name = "kActionShieldChange"; break;
			case DEFAULT_OBJECT::kActionPathStart: name = "kActionPathStart"; break;
			case DEFAULT_OBJECT::kActionPathEnd: name = "kActionPathEnd"; break;
			case DEFAULT_OBJECT::kActionLargeMovementDelta: name = "kActionLargeMovementDelta"; break;
			case DEFAULT_OBJECT::kActionFlyStart: name = "kActionFlyStart"; break;
			case DEFAULT_OBJECT::kActionFlyStop: name = "kActionFlyStop"; break;
			case DEFAULT_OBJECT::kActionHoverStart: name = "kActionHoverStart"; break;
			case DEFAULT_OBJECT::kActionHoverStop: name = "kActionHoverStop"; break;
			case DEFAULT_OBJECT::kActionBumpedInto: name = "kActionBumpedInto"; break;
			case DEFAULT_OBJECT::kActionSummonedStart: name = "kActionSummonedStart"; break;
			case DEFAULT_OBJECT::kActionTalkingIdle: name = "kActionTalkingIdle"; break;
			case DEFAULT_OBJECT::kActionListenIdle: name = "kActionListenIdle"; break;
			case DEFAULT_OBJECT::kActionDeath: name = "kActionDeath"; break;
			case DEFAULT_OBJECT::kActionDeathWait: name = "kActionDeathWait"; break;
			case DEFAULT_OBJECT::kActionIdleWarn: name = "kActionIdleWarn"; break;
			case DEFAULT_OBJECT::kActionMoveStart: name = "kActionMoveStart"; break;
			case DEFAULT_OBJECT::kActionMoveStop: name = "kActionMoveStop"; break;
			case DEFAULT_OBJECT::kActionTurnRight: name = "kActionTurnRight"; break;
			case DEFAULT_OBJECT::kActionTurnLeft: name = "kActionTurnLeft"; break;
			case DEFAULT_OBJECT::kActionTurnStop: name = "kActionTurnStop"; break;
			case DEFAULT_OBJECT::kActionMoveForward: name = "kActionMoveForward"; break;
			case DEFAULT_OBJECT::kActionMoveBackward: name = "kActionMoveBackward"; break;
			case DEFAULT_OBJECT::kActionMoveLeft: name = "kActionMoveLeft"; break;
			case DEFAULT_OBJECT::kActionMoveRight: name = "kActionMoveRight"; break;
			case DEFAULT_OBJECT::kActionResetAnimationGraph: name = "kActionResetAnimationGraph"; break;
			case DEFAULT_OBJECT::kActionKnockdown: name = "kActionKnockdown"; break;
			case DEFAULT_OBJECT::kActionGetUp: name = "kActionGetUp"; break;
			case DEFAULT_OBJECT::kActionIdleStopInstant: name = "kActionIdleStopInstant"; break;
			case DEFAULT_OBJECT::kActionRagdollInstant: name = "kActionRagdollInstant"; break;
			case DEFAULT_OBJECT::kActionWaterwalkStart: name = "kActionWaterwalkStart"; break;
			case DEFAULT_OBJECT::kActionReload: name = "kActionReload"; break;
			case DEFAULT_OBJECT::kPickupSoundGeneric: name = "kPickupSoundGeneric"; break;
			case DEFAULT_OBJECT::kPutdownSoundGeneric: name = "kPutdownSoundGeneric"; break;
			case DEFAULT_OBJECT::kPickupSoundWeapon: name = "kPickupSoundWeapon"; break;
			case DEFAULT_OBJECT::kPutdownSoundWeapon: name = "kPutdownSoundWeapon"; break;
			case DEFAULT_OBJECT::kPickupSoundArmor: name = "kPickupSoundArmor"; break;
			case DEFAULT_OBJECT::kPutdownSoundArmor: name = "kPutdownSoundArmor"; break;
			case DEFAULT_OBJECT::kPickupSoundBook: name = "kPickupSoundBook"; break;
			case DEFAULT_OBJECT::kPutdownSoundBook: name = "kPutdownSoundBook"; break;
			case DEFAULT_OBJECT::kPickupSoundIngredient: name = "kPickupSoundIngredient"; break;
			case DEFAULT_OBJECT::kPutdownSoundIngredient: name = "kPutdownSoundIngredient"; break;
			case DEFAULT_OBJECT::kHarvestSound: name = "kHarvestSound"; break;
			case DEFAULT_OBJECT::kHarvestFailedSound: name = "kHarvestFailedSound"; break;
			case DEFAULT_OBJECT::kWardBreakSound: name = "kWardBreakSound"; break;
			case DEFAULT_OBJECT::kWardAbsorbSound: name = "kWardAbsorbSound"; break;
			case DEFAULT_OBJECT::kWardDeflectSound: name = "kWardDeflectSound"; break;
			case DEFAULT_OBJECT::kMagicFailSound: name = "kMagicFailSound"; break;
			case DEFAULT_OBJECT::kShoutFailSound: name = "kShoutFailSound"; break;
			case DEFAULT_OBJECT::kHeartbeatSoundFast: name = "kHeartbeatSoundFast"; break;
			case DEFAULT_OBJECT::kHeartbeatSoundSlow: name = "kHeartbeatSoundSlow"; break;
			case DEFAULT_OBJECT::kImagespaceLowHealth: name = "kImagespaceLowHealth"; break;
			case DEFAULT_OBJECT::kSoulCapturedSound: name = "kSoulCapturedSound"; break;
			case DEFAULT_OBJECT::kNoActivationSound: name = "kNoActivationSound"; break;
			case DEFAULT_OBJECT::kMapMenuLoopingSound: name = "kMapMenuLoopingSound"; break;
			case DEFAULT_OBJECT::kDialogueVoiceCategory: name = "kDialogueVoiceCategory"; break;
			case DEFAULT_OBJECT::kNonDialogueVoiceCategory: name = "kNonDialogueVoiceCategory"; break;
			case DEFAULT_OBJECT::kSFXToFadeInDialogueCategory: name = "kSFXToFadeInDialogueCategory"; break;
			case DEFAULT_OBJECT::kPauseDuringMenuCategoryFade: name = "kPauseDuringMenuCategoryFade"; break;
			case DEFAULT_OBJECT::kPauseDuringMenuCategoryImmediate: name = "kPauseDuringMenuCategoryImmediate"; break;
			case DEFAULT_OBJECT::kPauseDuringLoadingMenuCategory: name = "kPauseDuringLoadingMenuCategory"; break;
			case DEFAULT_OBJECT::kMusicSoundCategory: name = "kMusicSoundCategory"; break;
			case DEFAULT_OBJECT::kStatsMuteCategory: name = "kStatsMuteCategory"; break;
			case DEFAULT_OBJECT::kStatsMusic: name = "kStatsMusic"; break;
			case DEFAULT_OBJECT::kMasterSoundCategory: name = "kMasterSoundCategory"; break;
			case DEFAULT_OBJECT::kTimeSensitiveSoundCategory: name = "kTimeSensitiveSoundCategory"; break;
			case DEFAULT_OBJECT::kDialogueOutputModel3D: name = "kDialogueOutputModel3D"; break;
			case DEFAULT_OBJECT::kDialogueOutputModel2D: name = "kDialogueOutputModel2D"; break;
			case DEFAULT_OBJECT::kPlayersOutputModel1stPerson: name = "kPlayersOutputModel1stPerson"; break;
			case DEFAULT_OBJECT::kPlayersOutputModel3rdPerson: name = "kPlayersOutputModel3rdPerson"; break;
			case DEFAULT_OBJECT::kInterfaceOutputModel: name = "kInterfaceOutputModel"; break;
			case DEFAULT_OBJECT::kReverbType: name = "kReverbType"; break;
			case DEFAULT_OBJECT::kUnderwaterLoopSound: name = "kUnderwaterLoopSound"; break;
			case DEFAULT_OBJECT::kUnderwaterReverbType: name = "kUnderwaterReverbType"; break;
			case DEFAULT_OBJECT::kKeywordHorse: name = "kKeywordHorse"; break;
			case DEFAULT_OBJECT::kKeywordUndead: name = "kKeywordUndead"; break;
			case DEFAULT_OBJECT::kKeywordNPC: name = "kKeywordNPC"; break;
			case DEFAULT_OBJECT::kKeywordBeastRace: name = "kKeywordBeastRace"; break;
			case DEFAULT_OBJECT::kKeywordDummyObject: name = "kKeywordDummyObject"; break;
			case DEFAULT_OBJECT::kKeywordUseGeometryEmitter: name = "kKeywordUseGeometryEmitter"; break;
			case DEFAULT_OBJECT::kKeywordMustStop: name = "kKeywordMustStop"; break;
			case DEFAULT_OBJECT::kKeywordUpdateDuringArchery: name = "kKeywordUpdateDuringArchery"; break;
			case DEFAULT_OBJECT::kKeywordSkipOutfitItems: name = "kKeywordSkipOutfitItems"; break;
			case DEFAULT_OBJECT::kMaleFaceTextureSetHead: name = "kMaleFaceTextureSetHead"; break;
			case DEFAULT_OBJECT::kMaleFaceTextureSetMouth: name = "kMaleFaceTextureSetMouth"; break;
			case DEFAULT_OBJECT::kMaleFaceTextureSetEyes: name = "kMaleFaceTextureSetEyes"; break;
			case DEFAULT_OBJECT::kFemaleFaceTextureSetHead: name = "kFemaleFaceTextureSetHead"; break;
			case DEFAULT_OBJECT::kFemaleFaceTextureSetMouth: name = "kFemaleFaceTextureSetMouth"; break;
			case DEFAULT_OBJECT::kFemaleFaceTextureSetEyes: name = "kFemaleFaceTextureSetEyes"; break;
			case DEFAULT_OBJECT::kImageSpaceModifierforinventorymenu: name = "kImageSpaceModifierforinventorymenu"; break;
			case DEFAULT_OBJECT::kPackagetemplate: name = "kPackagetemplate"; break;
			case DEFAULT_OBJECT::kMainMenuCell: name = "kMainMenuCell"; break;
			case DEFAULT_OBJECT::kDefaultMovementTypeWalk: name = "kDefaultMovementTypeWalk"; break;
			case DEFAULT_OBJECT::kDefaultMovementTypeRun: name = "kDefaultMovementTypeRun"; break;
			case DEFAULT_OBJECT::kDefaultMovementTypeSwim: name = "kDefaultMovementTypeSwim"; break;
			case DEFAULT_OBJECT::kDefaultMovementTypeFly: name = "kDefaultMovementTypeFly"; break;
			case DEFAULT_OBJECT::kDefaultMovementTypeSneak: name = "kDefaultMovementTypeSneak"; break;
			case DEFAULT_OBJECT::kDefaultMovementTypeSprint: name = "kDefaultMovementTypeSprint"; break;
			case DEFAULT_OBJECT::kKeywordSpecialFurniture: name = "kKeywordSpecialFurniture"; break;
			case DEFAULT_OBJECT::kKeywordFurnitureForces1stPerson: name = "kKeywordFurnitureForces1stPerson"; break;
			case DEFAULT_OBJECT::kKeywordFurnitureForces3rdPerson: name = "kKeywordFurnitureForces3rdPerson"; break;
			case DEFAULT_OBJECT::kKeywordActivatorFurnitureNoPlayer: name = "kKeywordActivatorFurnitureNoPlayer"; break;
			default: name = std::format("Other: {}", stl::to_underlying(v)); break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};