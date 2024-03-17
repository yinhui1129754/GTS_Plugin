#include "managers/GtsSizeManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "scale/modscale.hpp"
#include "data/plugin.hpp"


using namespace SKSE;
using namespace RE;

namespace {
	inline const auto ActorDataRecord = _byteswap_ulong('ACTD');
	inline const auto ScaleMethodRecord = _byteswap_ulong('SCMD');
	inline const auto HighHeelCorrectionRecord = _byteswap_ulong('HHCO');
	inline const auto HighHeelFurnitureRecord = _byteswap_ulong('HHFO');
	inline const auto SizeRaycastRecord = _byteswap_ulong('SREB');
	inline const auto AllowPlayerVoreRecord = _byteswap_ulong('APVR');
	inline const auto AllowInsectVoreRecord = _byteswap_ulong('AIVR');
	inline const auto AllowUndeadVoreRecord = _byteswap_ulong('AUVR');
	inline const auto AllowFollowerInteractions = _byteswap_ulong('AVFI');
	inline const auto FollowerProtectionRecord = _byteswap_ulong('FPRD');
	inline const auto DevourmentCompatRecord = _byteswap_ulong('DVCR');
	inline const auto FeetTrackingRecord = _byteswap_ulong('FTRD');
	inline const auto LessGoreRecord = _byteswap_ulong('LGRD');
	inline const auto AllowStaggerRecord = _byteswap_ulong('ASRD');
	inline const auto VoreCombatOnlyRecord = _byteswap_ulong('VRCO');
	inline const auto IsSpeedAdjustedRecord = _byteswap_ulong('ANAJ');
	inline const auto TremorScales = _byteswap_ulong('TREM');
	inline const auto CamCollisions = _byteswap_ulong('CAMC');
	inline const auto SizeDamageMult = _byteswap_ulong('SZDM');
	inline const auto XpMult = _byteswap_ulong('XPMT');
	inline const auto StompAiRecord = _byteswap_ulong('STAI');
	inline const auto SandwichAiRecord = _byteswap_ulong('SWAI');
	inline const auto KickAiRecord = _byteswap_ulong('KKAI');
	inline const auto HugsAiRecord = _byteswap_ulong('HSAI');
	inline const auto ButtAiRecord = _byteswap_ulong('BTAI');
	inline const auto VoreAiRecord = _byteswap_ulong('VRAI');
	inline const auto ProgressionMult = _byteswap_ulong('PRMT');
	inline const auto DeleteActors = _byteswap_ulong('DTAS');
	inline const auto HostileToggle = _byteswap_ulong('HTTL');
	inline const auto LegacySounds = _byteswap_ulong('LGSD');
	inline const auto ActorsPanic = _byteswap_ulong('ACTP');
	inline const auto LaunchObjects = _byteswap_ulong('LOBj');
	inline const auto CameraFovEdits = _byteswap_ulong('CFET');
	inline const auto NPC_EffectImmunity = _byteswap_ulong('NPER');
	inline const auto PC_EffectImmunity = _byteswap_ulong('PCER');

	inline const auto EnableIconsRecord = _byteswap_ulong('EIRC');

	inline const auto StolenAttributes = _byteswap_ulong('STAT');
	inline const auto Att_HealthStorage = _byteswap_ulong('HTSG');
	inline const auto Att_StaminStorage = _byteswap_ulong('STSG');
	inline const auto Att_MagickStorage = _byteswap_ulong('MTSG');

	// Quest
	inline const auto Record_StolenSize = _byteswap_ulong('QSSR');
	inline const auto Record_CrushCount = _byteswap_ulong('QCCR');
	inline const auto Record_STNCount = _byteswap_ulong('QSTR');
	inline const auto Record_HugStealCount = _byteswap_ulong('QHSR');
	inline const auto Record_HandCrushed = _byteswap_ulong('QHCR');
	inline const auto Record_VoreCount = _byteswap_ulong('QVRR');
	inline const auto Record_GiantCount = _byteswap_ulong('QGCR');
	//


	const float DEFAULT_MAX_SCALE = 65535.0;
	const float DEFAULT_HALF_LIFE = 1.0;
}

namespace Gts {
	Persistent& Persistent::GetSingleton() noexcept {
		static Persistent instance;
		return instance;
	}

	std::string Persistent::DebugName() {
		return "Persistent";
	}

	void Persistent::Reset() {
		//Plugin::SetInGame(false);
		std::unique_lock lock(this->_lock);
		this->_actor_data.clear();

    // Ensure we reset them back to inital scales
    // if they are loaded into game memory
    // since skyrim only lazy loads actors
    // that are already in memory it won't reload
    // their nif scales otherwise
    for (auto actor: find_actors()) {
      ResetToInitScale(actor);
    }
	}

	void Persistent::OnRevert(SerializationInterface*) {
		GetSingleton().Reset();
	}

	void Persistent::OnGameLoaded(SerializationInterface* serde) {
		std::uint32_t type;
		std::uint32_t size;
		std::uint32_t version;

		SizeManager::GetSingleton().Reset();

		FixAnimationsAndCamera(); // Call it from ActorUtils, needed to fix Grab anim on save-reload

		while (serde->GetNextRecordInfo(type, version, size)) {
			if (type == ActorDataRecord) {
				if (version >= 1) {
					std::size_t count;
					serde->ReadRecordData(&count, sizeof(count));
					for (; count > 0; --count) {
						RE::FormID actorFormID;
						serde->ReadRecordData(&actorFormID, sizeof(actorFormID));
						RE::FormID newActorFormID;
						if (!serde->ResolveFormID(actorFormID, newActorFormID)) {
							log::warn("Actor ID {:X} could not be found after loading the save.", actorFormID);
							continue;
						}
						float native_scale;
						serde->ReadRecordData(&native_scale, sizeof(native_scale));
						if (std::isnan(native_scale)) {
							native_scale = 1.0;
						}

						float visual_scale;
						serde->ReadRecordData(&visual_scale, sizeof(visual_scale));
						if (std::isnan(visual_scale)) {
							visual_scale = 1.0;
						}

						float visual_scale_v;
						serde->ReadRecordData(&visual_scale_v, sizeof(visual_scale_v));
						if (std::isnan(visual_scale_v)) {
							visual_scale_v = 0.0;
						}

						float target_scale;
						serde->ReadRecordData(&target_scale, sizeof(target_scale));
						if (std::isnan(target_scale)) {
							target_scale = 1.0;
						}

						float max_scale;
						serde->ReadRecordData(&max_scale, sizeof(max_scale));
						if (std::isnan(max_scale)) {
							max_scale = DEFAULT_MAX_SCALE;
						}

						float half_life;
						if (version >= 2) {
							serde->ReadRecordData(&half_life, sizeof(half_life));
						} else {
							half_life = DEFAULT_HALF_LIFE;
						}
						if (std::isnan(half_life)) {
							half_life = DEFAULT_HALF_LIFE;
						}

						float anim_speed;
						if (version >= 3) {
							serde->ReadRecordData(&anim_speed, sizeof(anim_speed));
						} else {
							anim_speed = 1.0;
						}
						if (std::isnan(anim_speed)) {
							anim_speed = 1.0;
						}

						float effective_multi;
						if (version >= 4) {
							serde->ReadRecordData(&effective_multi, sizeof(effective_multi));
						} else {
							effective_multi = 1.0;
						}
						if (std::isnan(effective_multi)) {
							effective_multi = 1.0;
						}

						float bonus_hp;
						if (version >= 5) {
							serde->ReadRecordData(&bonus_hp, sizeof(bonus_hp));
						} else {
							bonus_hp = 0.0;
						}
						if (std::isnan(bonus_hp)) {
							bonus_hp = 0.0;
						}

						float bonus_carry;
						if (version >= 5) {
							serde->ReadRecordData(&bonus_carry, sizeof(bonus_carry));
						} else {
							bonus_carry = 0.0;
						}
						if (std::isnan(bonus_carry)) {
							bonus_carry = 0.0;
						}

						float bonus_max_size;
						if (version >= 5) {
							serde->ReadRecordData(&bonus_max_size, sizeof(bonus_max_size));
						} else {
							bonus_max_size = 0.0;
						}
						if (std::isnan(bonus_max_size)) {
							bonus_max_size = 0.0;
						}
						float smt_run_speed;
						if (version >= 6) {
							serde->ReadRecordData(&smt_run_speed, sizeof(smt_run_speed));
						} else {
							smt_run_speed = 0.0;
						}
						if (std::isnan(smt_run_speed)) {
							smt_run_speed = 0.0;
						}

						float NormalDamage; //0
						if (version >= 6) {
							serde->ReadRecordData(&NormalDamage, sizeof(NormalDamage));
						} else {
							NormalDamage = 0.0;
						}
						if (std::isnan(NormalDamage)) {
							NormalDamage = 0.0;
						}

						float SprintDamage; //1
						if (version >= 6) {
							serde->ReadRecordData(&SprintDamage, sizeof(SprintDamage));
						} else {
							SprintDamage = 0.0;
						}
						if (std::isnan(SprintDamage)) {
							SprintDamage = 0.0;
						}

						float FallDamage; //2
						if (version >= 6) {
							serde->ReadRecordData(&FallDamage, sizeof(FallDamage));
						} else {
							FallDamage = 0.0;
						}
						if (std::isnan(FallDamage)) {
							FallDamage = 0.0;
						}

						float HHDamage; //3
						if (version >= 6) {
							serde->ReadRecordData(&HHDamage, sizeof(HHDamage));
						} else {
							HHDamage = 0.0;
						}
						if (std::isnan(HHDamage)) {
							HHDamage = 0.0;
						}

						float SizeVulnerability;
						if (version >= 6) {
							serde->ReadRecordData(&SizeVulnerability, sizeof(SizeVulnerability));
						} else {
							SizeVulnerability = 0.0;
						}
						if (std::isnan(SizeVulnerability)) {
							SizeVulnerability = 0.0;
						}

						float AllowHitGrowth;
						if (version >= 6) {
							serde->ReadRecordData(&AllowHitGrowth, sizeof(AllowHitGrowth));
						} else {
							AllowHitGrowth = 1.0;
						}
						if (std::isnan(AllowHitGrowth)) {
							AllowHitGrowth = 0.0;
						}

						float SizeReserve;
						if (version >= 6) {
							serde->ReadRecordData(&SizeReserve, sizeof(SizeReserve));
						} else {
							SizeReserve = 0.0;
						}
						if (std::isnan(SizeReserve)) {
							SizeReserve = 0.0;
						}


						float target_scale_v;
						if (version >= 7) {
							serde->ReadRecordData(&target_scale_v, sizeof(target_scale_v));
						} else {
							target_scale_v = 0.0;
						}
						if (std::isnan(target_scale_v)) {
							target_scale_v = 0.0;
						}

						float scaleOverride;
						if (version >= 8) {
							serde->ReadRecordData(&scaleOverride, sizeof(scaleOverride));
						} else {
							scaleOverride = -1.0;
						}
						if (std::isnan(scaleOverride)) {
							scaleOverride = -1.0;
						}

						float stolen_attributes;
						if (version >= 8) {
							serde->ReadRecordData(&stolen_attributes, sizeof(stolen_attributes));
						} else {
							stolen_attributes = 0.0;
						}
						if (std::isnan(stolen_attributes)) {
							stolen_attributes = 0.0;
						}

						float stolen_health;
						if (version >= 8) {
							serde->ReadRecordData(&stolen_health, sizeof(stolen_health));
						} else {
							stolen_health = 0.0;
						}
						if (std::isnan(stolen_health)) {
							stolen_health = 0.0;
						}

						float stolen_magick;
						if (version >= 8) {
							serde->ReadRecordData(&stolen_magick, sizeof(stolen_magick));
						} else {
							stolen_magick = 0.0;
						}
						if (std::isnan(stolen_magick)) {
							stolen_magick = 0.0;
						}


						float stolen_stamin;
						if (version >= 8) {
							serde->ReadRecordData(&stolen_stamin, sizeof(stolen_stamin));
						} else {
							stolen_stamin = 0.0;
						}
						if (std::isnan(stolen_stamin)) {
							stolen_stamin = 0.0;
						}


						ActorData data = ActorData();
						log::info("Loading Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}, half_life: {}, anim_speed: {}, bonus_hp: {}, bonus_carry: {}", newActorFormID, native_scale, visual_scale, visual_scale_v, target_scale, max_scale, half_life, anim_speed, bonus_hp, bonus_carry);
						data.native_scale = native_scale;
						data.visual_scale = visual_scale;
						data.visual_scale_v = visual_scale_v;
						data.target_scale = target_scale;
						data.max_scale = max_scale;
						data.half_life = half_life;
						data.anim_speed = anim_speed;
						data.effective_multi = effective_multi;
						data.bonus_hp = bonus_hp;
						data.bonus_carry = bonus_carry;
						data.bonus_max_size = bonus_max_size;
						data.smt_run_speed = smt_run_speed;
						data.NormalDamage = NormalDamage;
						data.SprintDamage = SprintDamage;
						data.FallDamage = FallDamage;
						data.HHDamage = HHDamage;
						data.SizeVulnerability = SizeVulnerability;
						data.AllowHitGrowth = AllowHitGrowth;
						data.SizeReserve = SizeReserve;
						data.target_scale_v = target_scale_v;
						data.scaleOverride = scaleOverride;

						data.stolen_attributes = stolen_attributes;
						data.stolen_health = stolen_health;
						data.stolen_magick = stolen_magick;
						data.stolen_stamin = stolen_stamin;

						TESForm* actor_form = TESForm::LookupByID<Actor>(newActorFormID);
						if (actor_form) {
							Actor* actor = skyrim_cast<Actor*>(actor_form);
							if (actor) {
								GetSingleton()._actor_data.insert_or_assign(newActorFormID, data);
							} else {
								log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
							}
						} else {
							log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
						}
					}
				} else {
					log::info("Disregarding version 0 cosave info.");
				}
			} else if (type == ScaleMethodRecord) {
				int size_method;
				serde->ReadRecordData(&size_method, sizeof(size_method));
				switch (size_method) {
					case 0:
						GetSingleton().size_method = SizeMethod::ModelScale;
						break;
					case 1:
						GetSingleton().size_method = SizeMethod::RootScale;
						break;
					case 2:
						GetSingleton().size_method = SizeMethod::Hybrid;
						break;
					case 3:
						GetSingleton().size_method = SizeMethod::RefScale;
						break;
				}
			} else if (type == HighHeelCorrectionRecord) {
				bool highheel_correction;
				serde->ReadRecordData(&highheel_correction, sizeof(highheel_correction));
				GetSingleton().highheel_correction = highheel_correction;
			} else if (type == SizeRaycastRecord) {
				bool SizeRaycast_Enabled;
				serde->ReadRecordData(&SizeRaycast_Enabled, sizeof(SizeRaycast_Enabled));
				GetSingleton().SizeRaycast_Enabled = SizeRaycast_Enabled;
			} else if (type == HighHeelFurnitureRecord) {
				bool highheel_furniture;
				serde->ReadRecordData(&highheel_furniture, sizeof(highheel_furniture));
				GetSingleton().highheel_furniture = highheel_furniture;
			} else if (type == AllowPlayerVoreRecord) {
				bool vore_allowplayervore;
				serde->ReadRecordData(&vore_allowplayervore, sizeof(vore_allowplayervore));
				GetSingleton().vore_allowplayervore = vore_allowplayervore;
			} else if (type == AllowInsectVoreRecord) {
				bool AllowInsectVore;
				serde->ReadRecordData(&AllowInsectVore, sizeof(AllowInsectVore));
				GetSingleton().AllowInsectVore = AllowInsectVore;
			} else if (type == AllowUndeadVoreRecord) {
				bool AllowUndeadVore;
				serde->ReadRecordData(&AllowUndeadVore, sizeof(AllowUndeadVore));
				GetSingleton().AllowUndeadVore = AllowUndeadVore;
			} else if (type == AllowFollowerInteractions) {
				bool FollowerInteractions;
				serde->ReadRecordData(&FollowerInteractions, sizeof(FollowerInteractions));
				GetSingleton().FollowerInteractions = FollowerInteractions;
			} else if (type == FollowerProtectionRecord) {
				bool FollowerProtection;
				serde->ReadRecordData(&FollowerProtection, sizeof(FollowerProtection));
				GetSingleton().FollowerProtection = FollowerProtection;
			} else if (type == VoreCombatOnlyRecord) {
				bool vore_combatonly;
				serde->ReadRecordData(&vore_combatonly, sizeof(vore_combatonly));
				GetSingleton().vore_combatonly = vore_combatonly;
			} else if (type == DevourmentCompatRecord) {
				bool devourment_compatibility;
				serde->ReadRecordData(&devourment_compatibility, sizeof(devourment_compatibility));
				GetSingleton().devourment_compatibility = devourment_compatibility;
			} else if (type == FeetTrackingRecord) {
				bool allow_feetracking;
				serde->ReadRecordData(&allow_feetracking, sizeof(allow_feetracking));
				GetSingleton().allow_feetracking = allow_feetracking;
			} else if (type == LessGoreRecord) {
				bool less_gore;
				serde->ReadRecordData(&less_gore, sizeof(less_gore));
				GetSingleton().less_gore = less_gore;
			} else if (type == AllowStaggerRecord) {
				bool allow_stagger;
				serde->ReadRecordData(&allow_stagger, sizeof(allow_stagger));
				GetSingleton().allow_stagger = allow_stagger;
			} else if (type == StompAiRecord) {
				bool Stomp_Ai;
				serde->ReadRecordData(&Stomp_Ai, sizeof(Stomp_Ai));
				GetSingleton().Stomp_Ai = Stomp_Ai;
			} else if (type == DeleteActors) {
				bool delete_actors;
				serde->ReadRecordData(&delete_actors, sizeof(delete_actors));
				GetSingleton().delete_actors = delete_actors;
			} else if (type == LegacySounds) {
				bool legacy_sounds;
				serde->ReadRecordData(&legacy_sounds, sizeof(legacy_sounds));
				GetSingleton().legacy_sounds = legacy_sounds;
			} else if (type == ActorsPanic) {
				bool actors_panic;
				serde->ReadRecordData(&actors_panic, sizeof(actors_panic));
				GetSingleton().actors_panic = actors_panic;
			} else if (type == LaunchObjects) {
				bool launch_objects;
				serde->ReadRecordData(&launch_objects, sizeof(launch_objects));
				GetSingleton().launch_objects = launch_objects;
			} else if (type == CameraFovEdits) {
				bool Camera_PermitFovEdits;
				serde->ReadRecordData(&Camera_PermitFovEdits, sizeof(Camera_PermitFovEdits));
				GetSingleton().Camera_PermitFovEdits = Camera_PermitFovEdits;
			} else if (type == StolenAttributes) {
				float stolen_attributes;
				serde->ReadRecordData(&stolen_attributes, sizeof(stolen_attributes));
				GetSingleton().stolen_attributes = stolen_attributes;
			} else if (type == Att_HealthStorage) {
				float stolen_health;
				serde->ReadRecordData(&stolen_health, sizeof(stolen_health));
				GetSingleton().stolen_health = stolen_health;
			} else if (type == Att_MagickStorage) {
				float stolen_magick;
				serde->ReadRecordData(&stolen_magick, sizeof(stolen_magick));
				GetSingleton().stolen_magick = stolen_magick;
			} else if (type == Att_StaminStorage) {
				float stolen_stamin;
				serde->ReadRecordData(&stolen_stamin, sizeof(stolen_stamin));
				GetSingleton().stolen_stamin = stolen_stamin;
			}
			/////////////////////////////////////////////////////////////////////////// Quest
			else if (type == Record_StolenSize) { // stage 1
				float StolenSize;
				serde->ReadRecordData(&StolenSize, sizeof(StolenSize));
				GetSingleton().StolenSize = StolenSize;
			} else if (type == Record_CrushCount) { // stage 2
				float CrushCount;
				serde->ReadRecordData(&CrushCount, sizeof(CrushCount));
				GetSingleton().CrushCount = CrushCount;
			} else if (type == Record_STNCount) { // stage 3
				float STNCount;
				serde->ReadRecordData(&STNCount, sizeof(STNCount));
				GetSingleton().STNCount = STNCount;
			} else if (type == Record_HugStealCount) { // stage 4
				float HugStealCount;
				serde->ReadRecordData(&HugStealCount, sizeof(HugStealCount));
				GetSingleton().HugStealCount = HugStealCount;
			} else if (type == Record_HandCrushed) { // stage 5
				float HandCrushed;
				serde->ReadRecordData(&HandCrushed, sizeof(HandCrushed));
				GetSingleton().HandCrushed = HandCrushed;
			} else if (type == Record_VoreCount) { // stage 6
				float VoreCount;
				serde->ReadRecordData(&VoreCount, sizeof(VoreCount));
				GetSingleton().VoreCount = VoreCount;
			} else if (type == Record_GiantCount) { // stage 7
				float GiantCount;
				serde->ReadRecordData(&GiantCount, sizeof(GiantCount));
				GetSingleton().GiantCount = GiantCount;
			}
			///////////////////////////////////////////////////////////////////////////
			else if (type == HostileToggle) {
				bool hostile_toggle;
				serde->ReadRecordData(&hostile_toggle, sizeof(hostile_toggle));
				GetSingleton().hostile_toggle = hostile_toggle;
			} else if (type == SandwichAiRecord) {
				bool Sandwich_Ai;
				serde->ReadRecordData(&Sandwich_Ai, sizeof(Sandwich_Ai));
				GetSingleton().Sandwich_Ai = Sandwich_Ai;
			} else if (type == KickAiRecord) {
				bool Kick_Ai;
				serde->ReadRecordData(&Kick_Ai, sizeof(Kick_Ai));
				GetSingleton().Kick_Ai = Kick_Ai;
			} else if (type == ButtAiRecord) {
				bool Butt_Ai;
				serde->ReadRecordData(&Butt_Ai, sizeof(Butt_Ai));
				GetSingleton().Butt_Ai = Butt_Ai;
			} else if (type == HugsAiRecord) {
				bool Hugs_Ai;
				serde->ReadRecordData(&Hugs_Ai, sizeof(Hugs_Ai));
				GetSingleton().Hugs_Ai = Hugs_Ai;
			} else if (type == VoreAiRecord) {
				bool Vore_Ai;
				serde->ReadRecordData(&Vore_Ai, sizeof(Vore_Ai));
				GetSingleton().Vore_Ai = Vore_Ai;
			} else if (type == NPC_EffectImmunity) {
				bool NPCEffectImmunity;
				serde->ReadRecordData(&NPCEffectImmunity, sizeof(NPCEffectImmunity));
				GetSingleton().NPCEffectImmunity = NPCEffectImmunity;
			} else if (type == PC_EffectImmunity) {
				bool PCEffectImmunity;
				serde->ReadRecordData(&PCEffectImmunity, sizeof(PCEffectImmunity));
				GetSingleton().PCEffectImmunity = PCEffectImmunity;
			} else if (type == EnableIconsRecord) {
				bool EnableIcons;
				serde->ReadRecordData(&EnableIcons, sizeof(EnableIcons));
				GetSingleton().EnableIcons = EnableIcons;
			} else if (type == IsSpeedAdjustedRecord) {
				bool is_speed_adjusted;
				serde->ReadRecordData(&is_speed_adjusted, sizeof(is_speed_adjusted));
				GetSingleton().is_speed_adjusted = is_speed_adjusted;
				if (version >= 1) {
					float k;
					serde->ReadRecordData(&k, sizeof(k));
					GetSingleton().speed_adjustment.k = k;
					float n;
					serde->ReadRecordData(&n, sizeof(n));
					GetSingleton().speed_adjustment.n = n;
					float s;
					serde->ReadRecordData(&s, sizeof(s));
					GetSingleton().speed_adjustment.s = s;
					float o = 1.0;
					GetSingleton().speed_adjustment.o = o;
				}
			} else if (type == ProgressionMult) {
				float progression_multiplier;
				serde->ReadRecordData(&progression_multiplier, sizeof(progression_multiplier));
				GetSingleton().progression_multiplier = progression_multiplier;
			} else if (type == SizeDamageMult) {
				float size_related_damage_mult;
				serde->ReadRecordData(&size_related_damage_mult, sizeof(size_related_damage_mult));
				GetSingleton().size_related_damage_mult = size_related_damage_mult;
			} else if (type == XpMult) {
				float experience_mult;
				serde->ReadRecordData(&experience_mult, sizeof(experience_mult));
				GetSingleton().experience_mult = experience_mult;
			} else if (type == TremorScales) {
				float tremor_scale;
				serde->ReadRecordData(&tremor_scale, sizeof(tremor_scale));
				GetSingleton().tremor_scale = tremor_scale;
				float npc_tremor_scale;
				serde->ReadRecordData(&npc_tremor_scale, sizeof(npc_tremor_scale));
				GetSingleton().npc_tremor_scale = npc_tremor_scale;
			} else if (type == CamCollisions) {
				bool enable_trees;
				serde->ReadRecordData(&enable_trees, sizeof(enable_trees));
				GetSingleton().camera_collisions.enable_trees = enable_trees;
				bool enable_debris;
				serde->ReadRecordData(&enable_debris, sizeof(enable_debris));
				GetSingleton().camera_collisions.enable_debris = enable_debris;
				bool enable_terrain;
				serde->ReadRecordData(&enable_terrain, sizeof(enable_terrain));
				GetSingleton().camera_collisions.enable_terrain = enable_terrain;
				bool enable_actor;
				serde->ReadRecordData(&enable_actor, sizeof(enable_actor));
				GetSingleton().camera_collisions.enable_actor = enable_actor;
				if (version >= 1) {
					bool enable_static;
					serde->ReadRecordData(&enable_static, sizeof(enable_static));
					GetSingleton().camera_collisions.enable_static = enable_static;
				}
				float above_scale;
				serde->ReadRecordData(&above_scale, sizeof(above_scale));
				GetSingleton().camera_collisions.above_scale = above_scale;
			} else {
				log::warn("Unknown record type in cosave.");
				__assume(false);
			}
		}
	}

	void Persistent::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		if (!serde->OpenRecord(ActorDataRecord, 8)) {
			log::error("Unable to open actor data record to write cosave data.");
			return;
		}

		auto count = GetSingleton()._actor_data.size();
		serde->WriteRecordData(&count, sizeof(count));
		for (auto const& [form_id_t, data] : GetSingleton()._actor_data) {
			FormID form_id = form_id_t;
			float native_scale = data.native_scale;
			float visual_scale = data.visual_scale;
			float visual_scale_v = data.visual_scale_v;
			float target_scale = data.target_scale;
			float max_scale = data.max_scale;
			float half_life = data.half_life;
			float anim_speed = data.anim_speed;
			float effective_multi = data.effective_multi;
			float bonus_hp = data.bonus_hp;
			float bonus_carry = data.bonus_carry;
			float bonus_max_size = data.bonus_max_size;
			float smt_run_speed = data.smt_run_speed;
			float NormalDamage = data.NormalDamage;
			float SprintDamage = data.SprintDamage;
			float FallDamage = data.FallDamage;
			float HHDamage = data.HHDamage;
			float SizeVulnerability = data.SizeVulnerability;
			float AllowHitGrowth = data.AllowHitGrowth;
			float SizeReserve = data.SizeReserve;
			float target_scale_v = data.target_scale_v;
			float scaleOverride = data.scaleOverride;

			float stolen_attributes = data.stolen_attributes;
			float stolen_health = data.stolen_health;
			float stolen_magick = data.stolen_magick;
			float stolen_stamin = data.stolen_stamin;

			log::info("Saving Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}, half_life: {}, anim_speed: {}, effective_multi: {}, effective_multi: {}, bonus_hp: {}, bonus_carry: {}, bonus_max_size: {}", form_id, native_scale, visual_scale, visual_scale_v, target_scale, max_scale, half_life, anim_speed, effective_multi, effective_multi, bonus_hp, bonus_carry, bonus_max_size);
			serde->WriteRecordData(&form_id, sizeof(form_id));
			serde->WriteRecordData(&native_scale, sizeof(native_scale));
			serde->WriteRecordData(&visual_scale, sizeof(visual_scale));
			serde->WriteRecordData(&visual_scale_v, sizeof(visual_scale_v));
			serde->WriteRecordData(&target_scale, sizeof(target_scale));
			serde->WriteRecordData(&max_scale, sizeof(max_scale));
			serde->WriteRecordData(&half_life, sizeof(half_life));
			serde->WriteRecordData(&anim_speed, sizeof(anim_speed));
			serde->WriteRecordData(&effective_multi, sizeof(effective_multi));
			serde->WriteRecordData(&bonus_hp, sizeof(bonus_hp));
			serde->WriteRecordData(&bonus_carry, sizeof(bonus_carry));
			serde->WriteRecordData(&bonus_max_size, sizeof(bonus_max_size));
			serde->WriteRecordData(&smt_run_speed, sizeof(smt_run_speed));

			serde->WriteRecordData(&NormalDamage, sizeof(NormalDamage));
			serde->WriteRecordData(&SprintDamage, sizeof(SprintDamage));
			serde->WriteRecordData(&FallDamage, sizeof(FallDamage));
			serde->WriteRecordData(&HHDamage, sizeof(HHDamage));
			serde->WriteRecordData(&SizeVulnerability, sizeof(SizeVulnerability));
			serde->WriteRecordData(&AllowHitGrowth, sizeof(AllowHitGrowth));
			serde->WriteRecordData(&SizeReserve, sizeof(SizeReserve));

			serde->WriteRecordData(&target_scale_v, sizeof(target_scale_v));
			serde->WriteRecordData(&scaleOverride, sizeof(scaleOverride));

			serde->WriteRecordData(&stolen_attributes, sizeof(stolen_attributes));
			serde->WriteRecordData(&stolen_health, sizeof(stolen_health));
			serde->WriteRecordData(&stolen_magick, sizeof(stolen_magick));
			serde->WriteRecordData(&stolen_stamin, sizeof(stolen_stamin));
		}

		if (!serde->OpenRecord(ScaleMethodRecord, 0)) {
			log::error("Unable to open scale method record to write cosave data.");
			return;
		}

		int size_method = GetSingleton().size_method;
		serde->WriteRecordData(&size_method, sizeof(size_method));

		if (!serde->OpenRecord(HighHeelCorrectionRecord, 0)) {
			log::error("Unable to open high heel correction record to write cosave data.");
			return;
		}

		bool highheel_correction = GetSingleton().highheel_correction;
		serde->WriteRecordData(&highheel_correction, sizeof(highheel_correction));

		if (!serde->OpenRecord(SizeRaycastRecord, 0)) {
			log::error("Unable to open size raycast record to write cosave data.");
			return;
		}
		bool SizeRaycast_Enabled = GetSingleton().SizeRaycast_Enabled;
		serde->WriteRecordData(&SizeRaycast_Enabled, sizeof(SizeRaycast_Enabled));

		if (!serde->OpenRecord(HighHeelFurnitureRecord, 0)) {
			log::error("Unable to open high heel furniture record to write cosave data.");
			return;
		}

		bool highheel_furniture = GetSingleton().highheel_furniture;
		serde->WriteRecordData(&highheel_furniture, sizeof(highheel_furniture));


		if (!serde->OpenRecord(AllowPlayerVoreRecord, 0)) {
			log::error("Unable to open Allow Player Vore record to write cosave data.");
			return;
		}

		bool vore_allowplayervore = GetSingleton().vore_allowplayervore;
		serde->WriteRecordData(&vore_allowplayervore, sizeof(vore_allowplayervore));

		if (!serde->OpenRecord(AllowInsectVoreRecord, 0)) {
			log::error("Unable to open Allow Insect Vore record to write cosave data.");
			return;
		}
		bool AllowInsectVore = GetSingleton().AllowInsectVore;
		serde->WriteRecordData(&AllowInsectVore, sizeof(AllowInsectVore));

		if (!serde->OpenRecord(AllowUndeadVoreRecord, 0)) {
			log::error("Unable to open Allow Undead Vore record to write cosave data.");
			return;
		}
		bool AllowUndeadVore = GetSingleton().AllowUndeadVore;
		serde->WriteRecordData(&AllowUndeadVore, sizeof(AllowUndeadVore));

		if (!serde->OpenRecord(AllowFollowerInteractions, 0)) {
			log::error("Unable to open Follower Interactions record to write cosave data.");
			return;
		}
		bool FollowerInteractions = GetSingleton().FollowerInteractions;
		serde->WriteRecordData(&FollowerInteractions, sizeof(FollowerInteractions));

		if (!serde->OpenRecord(FollowerProtectionRecord, 0)) {
			log::error("Unable to open Follower Protection record to write cosave data.");
			return;
		}
		bool FollowerProtection = GetSingleton().FollowerProtection;
		serde->WriteRecordData(&FollowerProtection, sizeof(FollowerProtection));

		if (!serde->OpenRecord(VoreCombatOnlyRecord, 0)) {
			log::error("Unable to open Vore Combat Only record to write cosave data");
			return;
		}

		bool vore_combatonly = GetSingleton().vore_combatonly;
		serde->WriteRecordData(&vore_combatonly, sizeof(vore_combatonly));

		if (!serde->OpenRecord(DevourmentCompatRecord, 0)) {
			log::error("Unable to open Devourment Compatibility record to write cosave data");
			return;
		}

		bool devourment_compatibility = GetSingleton().devourment_compatibility;
		serde->WriteRecordData(&devourment_compatibility, sizeof(devourment_compatibility));

		if (!serde->OpenRecord(AllowStaggerRecord, 1)) {
			log::error("Unable to open Allow Stagger record to write cosave data");
			return;
		}
		bool allow_stagger = GetSingleton().allow_stagger;
		serde->WriteRecordData(&allow_stagger, sizeof(allow_stagger));

		if (!serde->OpenRecord(LessGoreRecord, 1)) {
			log::error("Unable to open Less Gore record to write cosave data");
			return;
		}
		bool less_gore = GetSingleton().less_gore;
		serde->WriteRecordData(&less_gore, sizeof(less_gore));

		if (!serde->OpenRecord(FeetTrackingRecord, 1)) {
			log::error("Unable to open Feet Tracking record to write cosave data.");
			return;
		}
		bool allow_feetracking = GetSingleton().allow_feetracking;
		serde->WriteRecordData(&allow_feetracking, sizeof(allow_feetracking));

		if (!serde->OpenRecord(StompAiRecord, 1)) {
			log::error("Unable to open Stomp Ai record to write cosave data.");
			return;
		}
		bool Stomp_Ai = GetSingleton().Stomp_Ai;
		serde->WriteRecordData(&Stomp_Ai, sizeof(Stomp_Ai));


		if (!serde->OpenRecord(LaunchObjects, 1)) {
			log::error("Unable to open Launch Objects record to write cosave data");
			return;
		}
		bool launch_objects = GetSingleton().launch_objects;
		serde->WriteRecordData(&launch_objects, sizeof(launch_objects));

		if (!serde->OpenRecord(CameraFovEdits, 1)) {
			log::error("Unable to open Camera For Permission record to write cosave data");
			return;
		}
		bool Camera_PermitFovEdits = GetSingleton().Camera_PermitFovEdits;
		serde->WriteRecordData(&Camera_PermitFovEdits, sizeof(Camera_PermitFovEdits));

		if (!serde->OpenRecord(StolenAttributes, 1)) {
			log::error("Unable to open Stolen Attributes record to write cosave data");
			return;
		}
		float stolen_attributes = GetSingleton().stolen_attributes;
		serde->WriteRecordData(&stolen_attributes, sizeof(stolen_attributes));


		if (!serde->OpenRecord(Att_HealthStorage, 1)) {
			log::error("Unable to open Stolen Health Attributes record to write cosave data");
			return;
		}
		float stolen_health = GetSingleton().stolen_health;
		serde->WriteRecordData(&stolen_health, sizeof(stolen_health));

		if (!serde->OpenRecord(Att_StaminStorage, 1)) {
			log::error("Unable to open Stolen Stamina Attributes record to write cosave data");
			return;
		}
		float stolen_stamin = GetSingleton().stolen_stamin;
		serde->WriteRecordData(&stolen_stamin, sizeof(stolen_stamin));

		if (!serde->OpenRecord(Att_MagickStorage, 1)) {
			log::error("Unable to open Stolen Stamina Attributes record to write cosave data");
			return;
		}
		float stolen_magick = GetSingleton().stolen_magick;
		serde->WriteRecordData(&stolen_magick, sizeof(stolen_magick));

		/////////////////////////////////////////////////////////////////////////////////////////// Quest

		if (!serde->OpenRecord(Record_StolenSize, 1)) { // Stage 1
			log::error("Unable to open Stage 1 record to write cosave data");
			return;
		}
		float StolenSize = GetSingleton().StolenSize;
		serde->WriteRecordData(&StolenSize, sizeof(StolenSize));

		if (!serde->OpenRecord(Record_CrushCount, 1)) { // Stage 2
			log::error("Unable to open Stage 2 record to write cosave data");
			return;
		}
		float CrushCount = GetSingleton().CrushCount;
		serde->WriteRecordData(&CrushCount, sizeof(CrushCount));

		if (!serde->OpenRecord(Record_STNCount, 1)) { // Stage 3
			log::error("Unable to open Stage 3 record to write cosave data");
			return;
		}
		float STNCount = GetSingleton().STNCount;
		serde->WriteRecordData(&STNCount, sizeof(STNCount));

		if (!serde->OpenRecord(Record_HugStealCount, 1)) { // Stage 4
			log::error("Unable to open Stage 4 record to write cosave data");
			return;
		}
		float HugStealCount = GetSingleton().HugStealCount;
		serde->WriteRecordData(&HugStealCount, sizeof(HugStealCount));

		if (!serde->OpenRecord(Record_HandCrushed, 1)) { // Stage 5
			log::error("Unable to open Stage 5 record to write cosave data");
			return;
		}
		float HandCrushed = GetSingleton().HandCrushed;
		serde->WriteRecordData(&HandCrushed, sizeof(HandCrushed));

		if (!serde->OpenRecord(Record_VoreCount, 1)) { // Stage 6
			log::error("Unable to open Stage 6 record to write cosave data");
			return;
		}
		float VoreCount = GetSingleton().VoreCount;
		serde->WriteRecordData(&VoreCount, sizeof(VoreCount));

		if (!serde->OpenRecord(Record_GiantCount, 1)) { // stage 7
			log::error("Unable to open Stage 7 record to write cosave data");
			return;
		}
		float GiantCount = GetSingleton().GiantCount;
		serde->WriteRecordData(&GiantCount, sizeof(GiantCount));

		///////////////////////////////////////////////////////////////////////////////////////////
		if (!serde->OpenRecord(ActorsPanic, 1)) {
			log::error("Unable to open Actors Panic record to write cosave data");
			return;
		}
		bool actors_panic = GetSingleton().actors_panic;
		serde->WriteRecordData(&actors_panic, sizeof(actors_panic));

		if (!serde->OpenRecord(LegacySounds, 1)) {
			log::error("Unable to open Legacy Sounds record to write cosave data");
			return;
		}
		bool legacy_sounds = GetSingleton().legacy_sounds;
		serde->WriteRecordData(&legacy_sounds, sizeof(legacy_sounds));

		if (!serde->OpenRecord(NPC_EffectImmunity, 1)) {
			log::error("Unable to open NPC Effect Immunity record to write cosave data");
			return;
		}
		bool NPCEffectImmunity = GetSingleton().NPCEffectImmunity;
		serde->WriteRecordData(&NPCEffectImmunity, sizeof(NPCEffectImmunity));

		if (!serde->OpenRecord(PC_EffectImmunity, 1)) {
			log::error("Unable to open PC Effect Immunity record to write cosave data");
			return;
		}
		bool PCEffectImmunity = GetSingleton().PCEffectImmunity;
		serde->WriteRecordData(&PCEffectImmunity, sizeof(PCEffectImmunity));

		if (!serde->OpenRecord(EnableIconsRecord, 1)) {
			log::error("Unable to open Toggle Icons record to write cosave data");
			return;
		}

		bool EnableIcons = GetSingleton().EnableIcons;
		serde->WriteRecordData(&EnableIcons, sizeof(EnableIcons));

		if (!serde->OpenRecord(HostileToggle, 1)) {
			log::error("Unable to open Hostile Toggle Actors record to write cosave data");
			return;
		}
		bool hostile_toggle = GetSingleton().hostile_toggle;
		serde->WriteRecordData(&hostile_toggle, sizeof(hostile_toggle));

		if (!serde->OpenRecord(DeleteActors, 1)) {
			log::error("Unable to open Delete Actors record to write cosave data");
			return;
		}
		bool delete_actors = GetSingleton().delete_actors;
		serde->WriteRecordData(&delete_actors, sizeof(delete_actors));

		if (!serde->OpenRecord(HugsAiRecord, 1)) {
			log::error("Unable to open Hugs Ai record to write cosave data");
			return;
		}
		bool Hugs_Ai = GetSingleton().Hugs_Ai;
		serde->WriteRecordData(&Hugs_Ai, sizeof(Hugs_Ai));

		if (!serde->OpenRecord(SandwichAiRecord, 1)) {
			log::error("Unable to open Sandwich Ai record to write cosave data.");
			return;
		}
		bool Sandwich_Ai = GetSingleton().Sandwich_Ai;
		serde->WriteRecordData(&Sandwich_Ai, sizeof(Sandwich_Ai));

		if (!serde->OpenRecord(ButtAiRecord, 1)) {
			log::error("Unable to open Butt Ai record to write cosave data.");
		}
		bool Butt_Ai = GetSingleton().Butt_Ai;
		serde->WriteRecordData(&Butt_Ai, sizeof(Butt_Ai));

		if (!serde->OpenRecord(KickAiRecord, 1)) {
			log::error("Unable to open Kick Ai record to write cosave data.");
			return;
		}
		bool Kick_Ai = GetSingleton().Kick_Ai;
		serde->WriteRecordData(&Kick_Ai, sizeof(Kick_Ai));

		if (!serde->OpenRecord(VoreAiRecord, 1)) {
			log::error("Unable to open Vore ai record to write cosave data.");
			return;
		}

		bool Vore_Ai = GetSingleton().Vore_Ai;
		serde->WriteRecordData(&Vore_Ai, sizeof(Vore_Ai));

		if (!serde->OpenRecord(IsSpeedAdjustedRecord, 1)) {
			log::error("Unable to open is speed adjusted record to write cosave data.");
			return;
		}

		bool is_speed_adjusted = GetSingleton().is_speed_adjusted;
		serde->WriteRecordData(&is_speed_adjusted, sizeof(is_speed_adjusted));
		float k = GetSingleton().speed_adjustment.k;
		serde->WriteRecordData(&k, sizeof(k));
		float n = GetSingleton().speed_adjustment.n;
		serde->WriteRecordData(&n, sizeof(n));
		float s = GetSingleton().speed_adjustment.s;
		serde->WriteRecordData(&s, sizeof(s));

		if (!serde->OpenRecord(ProgressionMult, 0)) {
			log::error("Unable to open Progression mult record to write cosave data");
			return;
		}
		float progression_multiplier = GetSingleton().progression_multiplier;
		serde->WriteRecordData(&progression_multiplier, sizeof(progression_multiplier));


		if (!serde->OpenRecord(XpMult, 0)) {
			log::error("Unable to open Experience Mult record to write cosave data");
			return;
		}

		float experience_mult = GetSingleton().experience_mult;
		serde->WriteRecordData(&experience_mult, sizeof(experience_mult));

		if (!serde->OpenRecord(SizeDamageMult, 0)) {
			log::error("Unable to open Damage mult record to write cosave data");
			return;
		}

		float size_related_damage_mult = GetSingleton().size_related_damage_mult;
		serde->WriteRecordData(&size_related_damage_mult, sizeof(size_related_damage_mult));

		if (!serde->OpenRecord(TremorScales, 0)) {
			log::error("Unable to open tremor scale record to write cosave data.");
			return;
		}

		float tremor_scale = GetSingleton().tremor_scale;
		serde->WriteRecordData(&tremor_scale, sizeof(tremor_scale));
		float npc_tremor_scale = GetSingleton().npc_tremor_scale;
		serde->WriteRecordData(&npc_tremor_scale, sizeof(npc_tremor_scale));

		if (!serde->OpenRecord(CamCollisions, 1)) {
			log::error("Unable to open camera collisions record to write cosave data.");
			return;
		}

		bool enable_trees = GetSingleton().camera_collisions.enable_trees;
		serde->WriteRecordData(&enable_trees, sizeof(enable_trees));
		bool enable_debris = GetSingleton().camera_collisions.enable_debris;
		serde->WriteRecordData(&enable_debris, sizeof(enable_debris));
		bool enable_terrain = GetSingleton().camera_collisions.enable_terrain;
		serde->WriteRecordData(&enable_terrain, sizeof(enable_terrain));
		bool enable_actor = GetSingleton().camera_collisions.enable_actor;
		serde->WriteRecordData(&enable_actor, sizeof(enable_actor));
		bool enable_static = GetSingleton().camera_collisions.enable_static;
		serde->WriteRecordData(&enable_static, sizeof(enable_static));
		float above_scale = GetSingleton().camera_collisions.above_scale;
		serde->WriteRecordData(&above_scale, sizeof(above_scale));
	}

	ActorData::ActorData() {
		// Uninit data
		// Make sure it is set elsewhere
	}
	ActorData::ActorData(Actor* actor) {
		// DEFAULT VALUES FOR NEW ACTORS
		auto scale = 1.0;//get_scale(actor);
		this->native_scale = scale;
		this->visual_scale = scale;
		this->visual_scale_v = 0.0;
		this->target_scale = scale;
		this->max_scale = DEFAULT_MAX_SCALE;
		this->half_life = DEFAULT_HALF_LIFE;
		this->anim_speed = 1.0;
		this->bonus_hp = 0.0;
		this->bonus_carry = 0.0;
		this->bonus_max_size = 0.0;
		this->smt_run_speed = 0.0;
		this->NormalDamage = 1.0;
		this->SprintDamage = 1.0;
		this->FallDamage = 1.0;
		this->HHDamage = 1.0;
		this->SizeVulnerability = 0.0;
		this->AllowHitGrowth = 1.0;
		this->SizeReserve = 0.0;
		this->scaleOverride = -1.0;

		this->stolen_attributes = 0.0;
		this->stolen_health = 0.0;
		this->stolen_magick = 0.0;
		this->stolen_stamin = 0.0;
	}

	ActorData* Persistent::GetActorData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		return this->GetActorData(*actor);
	}
	ActorData* Persistent::GetActorData(Actor& actor) {
		std::unique_lock lock(this->_lock);
		auto key = actor.formID;
		ActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Add new
			if (!actor.Is3DLoaded()) {
				return nullptr;
			}
			auto scale = get_scale(&actor);
			if (scale < 0.0) {
				return nullptr;
			}
			this->_actor_data.try_emplace(key, &actor);
			result = &this->_actor_data.at(key);
		}
		return result;
	}

	ActorData* Persistent::GetData(TESObjectREFR* refr) {
		if (!refr) {
			return nullptr;
		}
		return this->GetData(*refr);
	}
	ActorData* Persistent::GetData(TESObjectREFR& refr) {
		auto key = refr.formID;
		ActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			return nullptr;
		}
		return result;
	}

	void Persistent::ResetActor(Actor* actor) {
		// Fired after a TESReset event
		//  This event should be when the game attempts to reset their
		//  actor values etc when the cell resets
		auto data = this->GetData(actor);
		if (data) {
			// 10.12.2023: changed visual and target scale to 1.0 instead of data->native... stuff
			// Attempt to fix actors spawning with their old size basically
			data->visual_scale = 1.0; //data->native_scale;
			data->target_scale = 1.0; //data->native_scale;
			data->max_scale = DEFAULT_MAX_SCALE;
			data->visual_scale_v = 0.0;
			data->half_life = DEFAULT_HALF_LIFE;
			data->anim_speed = 1.0;
			data->bonus_hp = 0.0;
			data->bonus_carry = 0.0;
			data->bonus_max_size = 0.0;
			data->smt_run_speed = 0.0;
			data->NormalDamage = 1.0;
			data->SprintDamage = 1.0;
			data->FallDamage = 1.0;
			data->HHDamage = 1.0;
			data->SizeVulnerability = 0.0;
			data->AllowHitGrowth = 1.0;
			data->SizeReserve = 0.0;
			data->scaleOverride = -1.0;

			data->stolen_attributes = 0.0;
			data->stolen_health = 0.0;
			data->stolen_magick = 0.0;
			data->stolen_stamin = 0.0;
		}
    ResetToInitScale(actor);
	}
}
