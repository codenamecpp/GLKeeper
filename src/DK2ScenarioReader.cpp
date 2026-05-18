#include "stdafx.h"
#include "DK2ScenarioReader.h"

//////////////////////////////////////////////////////////////////////////

#define CHECK_BIT_ON(flags, bitIndex) (((flags) & (1U << bitIndex)) > 0)

//////////////////////////////////////////////////////////////////////////

#define DIVIDER_FLOAT 4096.0f
#define DIVIDER_DOUBLE 65536.0f

//////////////////////////////////////////////////////////////////////////

#define READ_FROM_FSTREAM(filestream, data) \
    if (!filestream.read(reinterpret_cast<char*>(&data), sizeof(data))) \
    { \
        return false; \
    }

#define READ_FSTREAM_DATATYPE(filestream, destination, dataType) \
    { \
        dataType _dataType; \
        if (!filestream.read(reinterpret_cast<char*>(&_dataType), sizeof(dataType))) \
        { \
            return false; \
        } \
        destination = static_cast<std::remove_reference<decltype(destination)>::type>(_dataType); \
    }

#define SKIP_FSTREAM_BYTES(filestream, numBytes) \
    { \
        if (!filestream.seekg(numBytes, std::ios::cur)) \
        { \
            return false; \
        } \
    }

#define SKIP_FSTREAM_DATATYPE(filestream, dataType) SKIP_FSTREAM_BYTES(filestream, sizeof(dataType))
#define SKIP_FSTREAM_U8(filestream)                 SKIP_FSTREAM_DATATYPE(filestream, uint8_t)
#define SKIP_FSTREAM_U16(filestream)                SKIP_FSTREAM_DATATYPE(filestream, uint16_t)
#define SKIP_FSTREAM_U32(filestream)                SKIP_FSTREAM_DATATYPE(filestream, uint32_t)
#define READ_FSTREAM_U8(filestream, destination)    READ_FSTREAM_DATATYPE(filestream, destination, uint8_t)
#define READ_FSTREAM_U16(filestream, destination)   READ_FSTREAM_DATATYPE(filestream, destination, uint16_t)
#define READ_FSTREAM_U32(filestream, destination)   READ_FSTREAM_DATATYPE(filestream, destination, uint32_t)
#define READ_FSTREAM_I8(filestream, destination)    READ_FSTREAM_DATATYPE(filestream, destination, int8_t)
#define READ_FSTREAM_I16(filestream, destination)   READ_FSTREAM_DATATYPE(filestream, destination, int16_t);
#define READ_FSTREAM_I32(filestream, destination)   READ_FSTREAM_DATATYPE(filestream, destination, int32_t);

//////////////////////////////////////////////////////////////////////////

enum 
{
    DKLD_GLOBALS            = 0,
    DKLD_MAP                = 100,
    DKLD_TERRAIN            = 110,
    DKLD_ROOMS              = 120,
    DKLD_TRAPS              = 130,
    DKLD_DOORS              = 140,
    DKLD_KEEPER_SPELLS      = 150,
    DKLD_CREATURE_SPELLS    = 160,
    DKLD_CREATURES          = 170,
    DKLD_PLAYERS            = 180,
    DKLD_THINGS             = 190,
    DKLD_TRIGGERS           = 210,
    DKLD_LEVEL              = 220,
    DKLD_VARIABLES          = 230,
    DKLD_OBJECTS            = 240,
    DKLD_EFFECT_ELEMENTS    = 250,
    DKLD_SHOTS              = 260,
    DKLD_EFFECTS            = 270
};

//////////////////////////////////////////////////////////////////////////

enum
{
    DK_OBJECT_THING             = 194,
    DK_TRAP_THING               = 195,
    DK_DOOR_THING               = 196,
    DK_ACTIONPOINT_THING        = 197,
    DK_NEUTRAL_CREATURE_THING   = 198,
    DK_GOOD_CREATURE_THING      = 199,
    DK_CREATURE_THING           = 200,
    DK_HEROPARTY_THING          = 201,
    DK_DEAD_BODY_THING          = 202,
    DK_EFFECT_GENERATOR_THING   = 203,
    DK_ROOM_THING               = 204,
    DK_CAMERA_THING             = 205,
};

//////////////////////////////////////////////////////////////////////////

// ArtResource flags
enum 
{
    ARTRESF_PLAYER_COLOURED                     = (0x000002UL),
    ARTRESF_ANIMATING_TEXTURE                   = (0x000004UL),
    ARTRESF_HAS_START_ANIMATION                 = (0x000008UL),
    ARTRESF_HAS_END_ANIMATION                   = (0x000010UL),
    ARTRESF_RANDOM_START_FRAME                  = (0x000020UL),
    ARTRESF_ORIGIN_AT_BOTTOM                    = (0x000040UL),
    ARTRESF_DOESNT_LOOP                         = (0x000080UL),
    ARTRESF_FLAT                                = (0x000100UL),
    ARTRESF_DOESNT_USE_PROGRESSIVE_MESH         = (0x000200UL),
    ARTRESF_USE_ANIMATING_TEXTURE_FOR_SELECTION = (0x010000UL),
    ARTRESF_PRELOAD                             = (0x020000UL),
    ARTRESF_BLOOD                               = (0x040000UL)
};

//////////////////////////////////////////////////////////////////////////

enum 
{
    TERRAINF_SOLID                              = (0x00000001UL),
    TERRAINF_IMPENETRABLE                       = (0x00000002UL),
    TERRAINF_OWNABLE                            = (0x00000004UL),
    TERRAINF_TAGGABLE                           = (0x00000008UL),
    TERRAINF_ATTACKABLE                         = (0x00000020UL),
    TERRAINF_TORCH                              = (0x00000040UL), // has torch?
    TERRAINF_WATER                              = (0x00000080UL),
    TERRAINF_LAVA                               = (0x00000100UL),
    TERRAINF_ALWAYS_EXPLORED                    = (0x00000200UL), // doesnt used
    TERRAINF_PLAYER_COLOURED_PATH               = (0x00000400UL),
    TERRAINF_PLAYER_COLOURED_WALL               = (0x00000800UL),
    TERRAINF_CONSTRUCTION_TYPE_WATER            = (0x00001000UL),
    TERRAINF_CONSTRUCTION_TYPE_QUAD             = (0x00002000UL),
    TERRAINF_UNEXPLORE_IF_DUG_BY_ANOTHER_PLAYER = (0x00004000UL), // doesnt used
    TERRAINF_FILL_INABLE                        = (0x00008000UL),
    TERRAINF_ALLOW_ROOM_WALLS                   = (0x00010000UL),
    TERRAINF_DECAY                              = (0x00020000UL),
    TERRAINF_RANDOM_TEXTURE                     = (0x00040000UL),

    TERRAINF_TERRAIN_COLOR_R                    = (0x00080000UL), // expands R channel of lightColor
    TERRAINF_TERRAIN_COLOR_G                    = (0x00100000UL), // expands G channel of lightColor
    TERRAINF_TERRAIN_COLOR_B                    = (0x00200000UL), // expands B channel of lightColor

    TERRAINF_DWARF_CAN_DIG_THROUGH              = (0x00800000UL),
    TERRAINF_REVEAL_THROUGH_FOG_OF_WAR          = (0x01000000UL),

    TERRAINF_AMBIENT_COLOR_R                    = (0x02000000UL), // expands R channel of ambientLight
    TERRAINF_AMBIENT_COLOR_G                    = (0x04000000UL), // expands G channel of ambientLight
    TERRAINF_AMBIENT_COLOR_B                    = (0x08000000UL), // expands B channel of ambientLight

    TERRAINF_LIGHT                              = (0x10000000UL),
    TERRAINF_AMBIENT_LIGHT                      = (0x20000000UL),
};

//////////////////////////////////////////////////////////////////////////

enum 
{
    ROOMF_PLACEABLE_ON_WATER                    = (0x0001UL),
    ROOMF_PLACEABLE_ON_LAVA                     = (0x0002UL),
    ROOMF_PLACEABLE_ON_LAND                     = (0x0004UL),
    ROOMF_HAS_WALLS                             = (0x0008UL),
    ROOMF_CENTRE                                = (0x0010UL), // Placement
    ROOMF_SPECIAL_TILES                         = (0x0020UL), // Placement
    ROOMF_NORMAL_TILES                          = (0x0040UL), // Placement
    ROOMF_BUILDABLE                             = (0x0080UL),
    ROOMF_SPECIAL_WALLS                         = (0x0100UL), // Placement
    ROOMF_ATTACKABLE                            = (0x0200UL),
    ROOMF_UNK_0x0400                            = (0x0400UL),
    ROOMF_UNK_0x0800                            = (0x0800UL),
    ROOMF_HAS_FLAME                             = (0x1000UL),
    ROOMF_IS_GOOD                               = (0x2000UL)
};

//////////////////////////////////////////////////////////////////////////

enum 
{
    OBJECTF_DIE_OVER_TIME                       = (0x0000001UL),
    OBJECTF_DIE_OVER_TIME_IF_NOT_IN_ROOM        = (0x0000002UL),
    OBJECTF_TYPE_SPECIAL                        = (0x0000004UL),
    OBJECTF_TYPE_SPELL_BOOK                     = (0x0000008UL),
    OBJECTF_TYPE_CRATE                          = (0x0000010UL),
    OBJECTF_TYPE_LAIR                           = (0x0000020UL),
    OBJECTF_TYPE_GOLD                           = (0x0000040UL),
    OBJECTF_TYPE_FOOD                           = (0x0000080UL),
    OBJECTF_CAN_BE_PICKED_UP                    = (0x0000100UL),
    OBJECTF_CAN_BE_SLAPPED                      = (0x0000200UL),
    OBJECTF_DIE_WHEN_SLAPPED                    = (0x0000400UL),
    OBJECTF_TYPE_LEVEL_GEM                      = (0x0001000UL),
    OBJECTF_CAN_BE_DROPPED_ON_ANY_LAND          = (0x0002000UL),
    OBJECTF_OBSTACLE                            = (0x0004000UL),
    OBJECTF_BOUNCE                              = (0x0008000UL),
    OBJECTF_BOULDER_CAN_ROLL_THROUGH            = (0x0010000UL),
    OBJECTF_BOULDER_DESTROYS                    = (0x0020000UL),
    OBJECTF_PILLAR                              = (0x0040000UL),
    OBJECTF_DOOR_KEY                            = (0x0100000UL),
    OBJECTF_DAMAGEABLE                          = (0x0200000UL),
    OBJECTF_HIGHLIGHTABLE                       = (0x0400000UL),
    OBJECTF_PLACEABLE                           = (0x0800000UL),
    OBJECTF_FIRST_PERSON_OBSTACLE               = (0x1000000UL),
    OBJECTF_SOLID_OBSTACLE                      = (0x2000000UL),
    OBJECTF_CAST_SHADOWS                        = (0x4000000UL)
};

//////////////////////////////////////////////////////////////////////////

using ScenarioVariableType = unsigned int;
// well known variable types
enum : ScenarioVariableType
{
    ScenarioVariableType_Null = 0,
    ScenarioVariableType_CreaturePool = 1,
    ScenarioVariableType_Availability = 2,
    ScenarioVariableType_EntranceGenerationSpeedSeconds = 3,
    ScenarioVariableType_ClaimTileHealth = 4, // value=825
    ScenarioVariableType_AttackTileHealth = 5,
    ScenarioVariableType_RepairTileHealth = 6,
    ScenarioVariableType_MineGoldHealth = 7,
    ScenarioVariableType_DigRockHealth = 8,
    ScenarioVariableType_DigOwnWallHealth = 9,
    ScenarioVariableType_DigEnemyWallHealth = 10,
    ScenarioVariableType_FillInHealth = 11,
    ScenarioVariableType_ReinforceWallHealth = 12,
    ScenarioVariableType_RepairWallHealth = 13,
    ScenarioVariableType_ConvertRoomHealth = 14,
    ScenarioVariableType_AttackRoomHealth = 15,
    ScenarioVariableType_RepairRoomHealth = 16,
    // 17
    ScenarioVariableType_ChickenGenerationTimePerHatchery = 18,
    // 19
    ScenarioVariableType_GoldMinedFromGems = 20,
    ScenarioVariableType_TimeBetweenReattemptingHungerSeconds = 21,
    ScenarioVariableType_TimeBetweenReattemptingSleepSeconds = 22,
    ScenarioVariableType_DeadBodyDiesAfterSeconds = 23,
    ScenarioVariableType_MaxGoldPerTreasuryTile = 24,
    ScenarioVariableType_DecomposeValueNeededForVampire = 25,
    ScenarioVariableType_CreatureDyingStateDurationSeconds = 26,
    ScenarioVariableType_SpecialIncreaseGoldAmount = 27,
    ScenarioVariableType_SpecialIncreaseManaAmount = 28,
    // 29
    ScenarioVariableType_ImpIdleDelayBeforeReevaluationSeconds = 30,
    ScenarioVariableType_DelayBeforePayReevaluationSeconds = 31,
    ScenarioVariableType_MaxGoldPileOutsideTreasury = 32,
    ScenarioVariableType_PayDayFrequencySeconds = 33,
    ScenarioVariableType_MotionlessWhileResearchingSeconds = 34,
    ScenarioVariableType_MotionlessWhileManufacturingSeconds = 35,
    ScenarioVariableType_StateCounterDelayBeforeReattemptEating = 36,
    ScenarioVariableType_StateCounterWhileGuarding = 37,
    ScenarioVariableType_MotionlessWhilePrayingSeconds = 38,
    ScenarioVariableType_MotionlessInPrisonSeconds = 39,
    ScenarioVariableType_MotionlessWhileTrainingSeconds = 40,
    ScenarioVariableType_MotionlessInCasinoSeconds = 41,
    ScenarioVariableType_ExcessManaDecreaseRatePerSecond = 42,
    ScenarioVariableType_SacrificesId = 43,
    // 44
    ScenarioVariableType_DefaultHorizontalTerrainWibble = 45,
    ScenarioVariableType_DefaultVerticalTerrainWibble = 46,
    ScenarioVariableType_RebelMaxFollowers = 47,
    ScenarioVariableType_RebelLeavingPercentage = 48,
    ScenarioVariableType_RebelBecomeGoodPercentage = 49,
    ScenarioVariableType_RebelDefectPercentage = 50,
    ScenarioVariableType_TimeBeforeFirstPayDaySeconds = 51,
    ScenarioVariableType_DefaultTorchLightRed = 52,
    ScenarioVariableType_DefaultTorchLightGreen = 53,
    ScenarioVariableType_DefaultTorchLightBlue = 54,
    ScenarioVariableType_PrisonModifyCreatureHealthPerSecond = 55,
    // 56
    // 57
    // 58
    // 59
    // 60
    // 61
    // 62
    // 63
    // 64
    ScenarioVariableType_CreatureStatsId = 65,
    // 66
    ScenarioVariableType_DefaultCellingHeightTiles = 67,
    ScenarioVariableType_PayDayCutOffTimeSeconds = 68,
    // 69
    ScenarioVariableType_CasinoModifyCreatureGoldPerSecond = 70,
    ScenarioVariableType_CasinoBigWinKeeperLossPercentage = 71,
    // 72
    ScenarioVariableType_CreaturesSupportedByFirstPortal = 73,
    ScenarioVariableType_GameTicks = 74,
    ScenarioVariableType_ModifyHealthOfCreatureInLairPerSecond = 75,
    ScenarioVariableType_ModifyAngerOfCreatureInLairPerSecond = 76,
    // 77
    ScenarioVariableType_ModifyAngerInCompanyOfHatedCreaturesPerSecond = 78,
    ScenarioVariableType_ForceAppliedToSlappedCreature = 79,
    // 80
    // 81
    ScenarioVariableType_CannotSleepModifyCreatureHealthPerSecond = 82,
    ScenarioVariableType_CannotLeaveModifyCreatureHealthPerSecond = 83,
    ScenarioVariableType_PlayerRescanIntervalSeconds = 84,
    ScenarioVariableType_PlayerRescanRange = 85,
    ScenarioVariableType_MaxFreeRangeChickensPerPlayer = 86,
    ScenarioVariableType_WoodBridgeLifeOnLavaSeconds = 87,
    ScenarioVariableType_CreatureSleepsWhenBelowPercentHealth = 88,
    // 89
    // 90
    // 91
    // 92
    ScenarioVariableType_DoorPickedTimeSeconds = 93,
    ScenarioVariableType_MinimumImpThreshold = 94,
    ScenarioVariableType_TimeBeforeFreeImpGeneratedSeconds = 95,
    ScenarioVariableType_CreatureStunnedTimeSeconds = 96,
    ScenarioVariableType_CreatureStunnedEffectDelaySeconds = 97,
    ScenarioVariableType_FirstPersonStatIncreasePercentage = 98,
    // 99
    ScenarioVariableType_ManaCostInFirstPersonPerSecond = 100,
    ScenarioVariableType_HealthDecreaseWhenOnFirePerSecond = 101,
    ScenarioVariableType_ModifyCreatureAngerWhilePrayingPerSecond = 102,
    // 103
    ScenarioVariableType_RoomSellValuePercentageOfCost = 104,
    ScenarioVariableType_DoorSellValuePercentageOfCost = 105,
    ScenarioVariableType_TrapSellValuePercentageOfCost = 106,
    // 107
    // 108
    // 109
    ScenarioVariableType_InsufficientManaImpEvaluationPeriodSeconds = 110,
    ScenarioVariableType_DeathWhenPossessingManaReduction = 111,
    ScenarioVariableType_PercentageOfManaGainedBySacrificing = 112,
    ScenarioVariableType_PossessionFriendlyStatIncreasePercentage = 113,
    ScenarioVariableType_PossessionFriendlyHealthIncreasePercentage = 114,
    ScenarioVariableType_ModifyCreatureHealthWhilePrayingPerSecond = 115,
    ScenarioVariableType_HypnotismManaDrainPerSecond = 116,
    // 117
    // 118
    // 119
    // 120
    // 121
    // 122
    // 123
    // 124
    ScenarioVariableType_LowManaWarningThreshold = 125,
    ScenarioVariableType_TrainingRoomMaxExperienceLevel = 126,
    ScenarioVariableType_StunnedDamageIncreasePercentage = 127,
    ScenarioVariableType_MaxNumberOfThingsInHand = 128,
    ScenarioVariableType_RubberBandAreaLimit = 129,
    ScenarioVariableType_GravityConstant = 130,
    ScenarioVariableType_DungeonHeartHealthRegenerationPerSecond = 131,
    ScenarioVariableType_EPLossSecondWhileNotTraining = 132,
    ScenarioVariableType_ModifyAngerWhileSolitaryInPitPerSecond = 133,
    ScenarioVariableType_ModifyAngerOfCombatPitVictor = 134,
    ScenarioVariableType_ModifyAngerOfPitSpectatorPerSecond = 135,
    ScenarioVariableType_ConvertNeutralRoomHealth = 136,
    // 137
    ScenarioVariableType_ManufacturePointLossWhileNoWorkersPerSecond = 138,
    ScenarioVariableType_CasinoBigWinnerIncreasedHappinessPersentage = 139,
    ScenarioVariableType_DecreaseEntrancePersentageOfLastSackedCreatureType = 140,
    ScenarioVariableType_IncreaseEntrancePersentageOfNewCreatureTypes = 141,
    ScenarioVariableType_IncreasedWorkRatePersentageOfPlayersTorturedCreatureTypes = 142,
    ScenarioVariableType_IncreasedWorkRatePersentageWhenCasinoBigWinOccurs = 143,
    ScenarioVariableType_ImpPopCountdownTimeSeconds = 144,
    ScenarioVariableType_NumberOfRandomWallDeteriorationsPerSecond = 145,
    ScenarioVariableType_WallDeteriorationDamage = 146,
    ScenarioVariableType_WallDamageFromShot = 147,
    ScenarioVariableType_PercentageChanceOfKillingCreatureInPit = 148,
    ScenarioVariableType_ModifyPlayerColdWhileTrainingPerSecond = 149,
    ScenarioVariableType_SkeletonArmyDurationSeconds = 150,
    ScenarioVariableType_OvercrowdingJailBreakPercentagePerExtraCreature = 151,
    ScenarioVariableType_MaxTimeInGuardRoomBeforeWanderingToGuardPostSeconds = 152,
    // 153
    ScenarioVariableType_AmountOfGoldStolenInFirstPerson = 154,
    ScenarioVariableType_BoulderDeteriorationDamagePercentagePerSecond = 155,
    ScenarioVariableType_DrunkenDurationSeconds = 156,
    ScenarioVariableType_BoulderInitialHealth = 157,
    ScenarioVariableType_BoulderSpeedTilesPerSecond = 158,
    ScenarioVariableType_BoulderToCreatureDamagePercentage = 159,
    ScenarioVariableType_BoulderSpeedToHealthRatio = 160,
    ScenarioVariableType_BoulderFearAmount = 161,
    ScenarioVariableType_BoulderFearRange = 162,
    ScenarioVariableType_DrunkChancePercentagePerDrink = 163,
    ScenarioVariableType_IncreasedWorkRatePecentageFromSlapping = 164,
    ScenarioVariableType_IncreasedWorkRateDurationFromSlappingSeconds = 165,
    ScenarioVariableType_DungeonHeartMaxManaIncrease = 166,
    ScenarioVariableType_DungeonHeartManaGenerationIncreasePerSecond = 167,
    ScenarioVariableType_TremorDamagePerTerrainTilePerSecond = 168,
    ScenarioVariableType_ExternalGuardingDurationSeconds = 169,
    ScenarioVariableType_MinimumJobDurationSeconds = 170,
    // 171
    // 172
    ScenarioVariableType_CreatureCriticalHealthPercentageOfMax = 173,
    ScenarioVariableType_DungeonHeartObjectHealth = 174,
    ScenarioVariableType_ClaimNeutralManaVaultHealth = 175,
    ScenarioVariableType_ClaimEnemyManaVaultHealth = 176,
    ScenarioVariableType_CreaturesSupportedPerAdditionalPortal = 177,
    ScenarioVariableType_MaxGoldPerDungeonHeartTile = 178,
    ScenarioVariableType_ImpExperienceGainPerSecond = 179,
    ScenarioVariableType_DungeonHeartClaimScanRadiusTiles = 180,
    ScenarioVariableType_StunTimePercentageIncreaseWhenHit = 181,
    ScenarioVariableType_CallToArmsFightDistanceTiles = 182,
    ScenarioVariableType_DungeonHeartReportingDistanceTiles = 183,
    ScenarioVariableType_DwarfDiggingMultiplier = 184,
    ScenarioVariableType_TriggerTrapTriggerSpeedTilesPerSecond = 185,
    // 186
    ScenarioVariableType_FirstPersonAccuracyAngleDegrees = 187,
    ScenarioVariableType_SpecialReceiveImpsAmount = 188,
    ScenarioVariableType_DungeonHeartCreatureGatheringDistanceTiles = 189,
    ScenarioVariableType_GuardRoomReportingDistanceTiles = 190,
    ScenarioVariableType_GuardPostReportingDistanceTiles = 191,
    ScenarioVariableType_CombatPitMaxExperienceLevel = 192,
    ScenarioVariableType_TimeInHandBeforeCreaturesBecomeAngrySeconds = 193,
    ScenarioVariableType_SlapWorkFasterDurationSeconds = 194,
    ScenarioVariableType_SlapSpeedUpDurationSeconds = 195,
    ScenarioVariableType_TortureSpeedUpDurationSeconds = 196,
    ScenarioVariableType_CasinoWinSpeedUpDurationSeconds = 197,
    ScenarioVariableType_CriticalHealthPercentageForHeroLair = 198,
    ScenarioVariableType_GamblingModifyCreatureMoneySmilesPerSecond = 199,
    ScenarioVariableType_GamblingModifyCreatureMoneyMoneyPerSecond = 200,
    ScenarioVariableType_GamblingModifyCreatureAngerSmilesPerSecond = 201,
    ScenarioVariableType_GamblingModifyCreatureAngerMoneyPerSecond = 202,
    ScenarioVariableType_GamblingDelayBeforeBecomingFearlessSeconds = 203,
    ScenarioVariableType_GamblingFearlessDurationSeconds = 204,
    ScenarioVariableType_GamblingCreatureJackpotChanceSmilesPerSecond = 205,
    ScenarioVariableType_GamblingCreatureJackpotChanceMoneyPerSecond = 206,
    ScenarioVariableType_GamblingJackpotPayTimeLimit = 207,
    ScenarioVariableType_GamblingJackpotPayTimeLimitMissedAngerPercentageModifier = 208,
    ScenarioVariableType_GamblingJackpotBasicAmount = 209,
    ScenarioVariableType_FirstPersonRechargeModifyPercentage = 210,
    ScenarioVariableType_BackOffMinimumDurationSeconds = 211,
    ScenarioVariableType_BackOffMaximumDurationSeconds = 212,
    ScenarioVariableType_GoodCreaturesChaseEnemyDurationSeconds = 213,
    ScenarioVariableType_EvilCreaturesChaseEnemyDurationSeconds = 214,
    ScenarioVariableType_MusicLevelTwoThreatThreshold = 215,
    ScenarioVariableType_MusicLevelThreeThreatThreshold = 216,
    ScenarioVariableType_MusicLevelFourThreatThreshold = 217,
    ScenarioVariableType_TimeBeforeDungeonHeartConstructionBegins = 218,
    ScenarioVariableType_DefaultTorchLightIntensity = 219,
    ScenarioVariableType_DefaultTorchLightRadiusTiles = 220,
    ScenarioVariableType_DefaultTorchLightHeightTiles = 221,
    ScenarioVariableType_MaximumManaThreshold = 222,
    ScenarioVariableType_MaximumGroupNumberOfFollowers = 223,
    ScenarioVariableType_CreatureFirstPersonId = 224,
    ScenarioVariableType_CombatPitMeleeDamageModifierPercentage = 225,
    ScenarioVariableType_UpgradedPossessionCostPercentage = 226,
    ScenarioVariableType_UpgradedCallToArmsCostPercentage = 227,
    ScenarioVariableType_MPDScoreHeroKilled = 228,
    ScenarioVariableType_MPDScoreLandOwned = 229,
    ScenarioVariableType_MPDScoreGoldSlabsMined = 230,
    ScenarioVariableType_MPDScoreItemManufactured = 231,
    ScenarioVariableType_MPDScoreCreatureEntered = 232,
    // 233
    ScenarioVariableType_TortureChanceOfDyingWhenConverted = 234,
    ScenarioVariableType_MaximumManaGainPerSecond = 235,
    ScenarioVariableType_ClaimScanLightRed = 236,
    ScenarioVariableType_ClaimScanLightGreen = 237,
    ScenarioVariableType_ClaimScanLightBlue = 238,
    ScenarioVariableType_PitPercentageDamageTakenOfNormalCombat = 239,
    ScenarioVariableType_BoulderSlapDamage = 240,
    ScenarioVariableType_LevelRating = 241,
    ScenarioVariableType_AverageTime = 242,
};

//////////////////////////////////////////////////////////////////////////

inline bool KWDParseENUM(int inputInt, ePlayerID& outputID)
{
    static const ePlayerID IDs[] =
    {
        ePlayerID_Null, // 0
        ePlayerID_Good, // 1
        ePlayerID_Neutral, // 2
        ePlayerID_Keeper1, // 3
        ePlayerID_Keeper2, // 4
        ePlayerID_Keeper3, // 5
        ePlayerID_Keeper4, // 6
    };
    if (inputInt < CountOf(IDs))
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputInt, eBridgeTerrain& outputID)
{
    static const eBridgeTerrain IDs[] =
    {
        eBridgeTerrain_Null, // 0
        eBridgeTerrain_Water, // 1
        eBridgeTerrain_Lava, // 2
    };
    if (inputInt < CountOf(IDs))
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputInt, eThingMaterialType& outputID)
{
    static const eThingMaterialType IDs[] =
    {
        eThingMaterialType_None, // 0
        eThingMaterialType_Flesh, // 1
        eThingMaterialType_Rock, // 2
        eThingMaterialType_Wood, // 3
        eThingMaterialType_Metal1, // 4
        eThingMaterialType_Metal2, // 5
        eThingMaterialType_Magic, // 6
        eThingMaterialType_Glass, // 7
    };
    if (inputInt < CountOf(IDs))
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputInt, eArtResource& outputID)
{
    static const eArtResource IDs[] =
    {
        eArtResource_Null, // 0
        eArtResource_Sprite, // 1
        eArtResource_Alpha, // 2
        eArtResource_AdditiveAlpha, // 3
        eArtResource_TerrainMesh, // 4
        eArtResource_Mesh, // 5
        eArtResource_AnimatingMesh, // 6
        eArtResource_ProceduralMesh, // 7
        eArtResource_MeshCollection, // 8
    };
    if (inputInt < CountOf(IDs))
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputInt, ePlayerType& outputID)
{
    static const ePlayerType IDs[] =
    {
        ePlayerType_Human, // 0
        ePlayerType_AI, // 1
    };
    if (inputInt < CountOf(IDs))
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputInt, eComputerAI& outputID)
{
    static const eComputerAI IDs[] =
    {
        eComputerAI_MasterKeeper, // 0
        eComputerAI_Conqueror, // 1
        eComputerAI_Psychotic, // 2
        eComputerAI_Stalwart, // 3
        eComputerAI_Greyman, // 4
        eComputerAI_Idiot, // 5
        eComputerAI_Guardian, // 6
        eComputerAI_ThickSkinned, // 7
        eComputerAI_Paranoid, // 8
    };
    if (inputInt < CountOf(IDs))
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputInt, eRoomTileConstruction& outputID)
{
    static const eRoomTileConstruction IDs[] =
    {
        eRoomTileConstruction_Complete,
        eRoomTileConstruction_Quad,
        eRoomTileConstruction_3_by_3,
        eRoomTileConstruction_3_by_3_Rotated,
        eRoomTileConstruction_Normal,
        eRoomTileConstruction_CenterPool,
        eRoomTileConstruction_DoubleQuad,
        eRoomTileConstruction_5_by_5_Rotated,
        eRoomTileConstruction_HeroGate,
        eRoomTileConstruction_HeroGateTile,
        eRoomTileConstruction_HeroGate_2_by_2,
        eRoomTileConstruction_HeroGateFrontend,
        eRoomTileConstruction_HeroGate_3_by_1,
    };

    if (inputInt == 0)
        return false;

    --inputInt; // shift down index
    if (inputInt < eRoomTileConstruction_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputValue, eDirection& outputValue)
{
    static const eDirection IDs[] =
    {
        eDirection_N,
        eDirection_NE,
        eDirection_E,
        eDirection_SE,
        eDirection_S,
        eDirection_SW,
        eDirection_W,
        eDirection_NW
    };
    if (inputValue < CountOf(IDs)) 
    {
        outputValue = IDs[inputValue];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputValue, eCreatureJobClass& outputValue)
{
    static const eCreatureJobClass IDs[] =
    {
        eCreatureJobClass_Worker,
        eCreatureJobClass_Thinker,
        eCreatureJobClass_Fighter,
        eCreatureJobClass_Scout,
    };
    if (inputValue < CountOf(IDs))
    {
        outputValue = IDs[inputValue];
        return true;
    }
    return false;
}

inline bool KWDParseENUM(int inputValue, eCreatureJob& outputValue)
{
    static const eCreatureJob IDs[] =
    {
        eCreatureJob_None,
        eCreatureJob_Sleep,
        eCreatureJob_Eat,
        eCreatureJob_Research,
        eCreatureJob_Train,
        eCreatureJob_Manufacture,
        eCreatureJob_Guard,
        eCreatureJob_Torture,
        eCreatureJob_Pray,
        eCreatureJob_Drink,
        eCreatureJob_Leave,
        eCreatureJob_DestroyEnemyRooms,
        eCreatureJob_DestroyWalls,
        eCreatureJob_StealGold,
        eCreatureJob_StealSpells,
        eCreatureJob_Sulk,
        eCreatureJob_Rebel,
        eCreatureJob_StealManufactureCrates,
        eCreatureJob_KillCreatures,
        eCreatureJob_KillPlayer,
        eCreatureJob_Tunnelling,
        eCreatureJob_COUNT, // 21
        eCreatureJob_Wait,
        eCreatureJob_SendToActionPoint,
        eCreatureJob_Explore,
        eCreatureJob_StealEnemyGold,
        eCreatureJob_CombatPitSpectate,
        eCreatureJob_JailBreak,
        eCreatureJob_COUNT,
    };

    outputValue = eCreatureJob_COUNT;
    if (inputValue < CountOf(IDs))
    {
        outputValue = IDs[inputValue];
    }
    return outputValue < eCreatureJob_COUNT;
}

inline bool KWDRoomThingToRoomType(int inputValue, RoomTypeId& outputValue)
{
    outputValue = RoomTypeId_Null;
    switch (inputValue)
    {
        case 12: outputValue = RoomTypeId_Portal; break;
        case 14: outputValue = RoomTypeId_DungeonHeart; break;
        case 33: outputValue = RoomTypeId_HeroGate_2x2; break;
        case 34: outputValue = RoomTypeId_HeroGate_Frontend; break;
        case 37: outputValue = RoomTypeId_HeroGate_3x1; break;
        case 40: outputValue = RoomTypeId_HeroPortal; break;
    }
    return outputValue != RoomTypeId_Null;
}

//////////////////////////////////////////////////////////////////////////

bool DK2ScenarioReader::ReadString(unsigned int stringLength, std::wstring& wideString)
{
    wideString.resize(stringLength);

    // read bytes
    if (!cxx::read_elements(mFileStream, wideString.data(), stringLength))
        return false;

    // trim
    wideString.erase(
        std::find_if(wideString.begin(), wideString.end(), [](wchar_t c) -> bool {
            return c == 0;
        }), 
        wideString.end());

    return true;
}

bool DK2ScenarioReader::ReadString8(unsigned int stringLength, std::string& ansiString)
{
    ansiString.resize(stringLength);

    // read bytes
    if (!mFileStream.read(&ansiString[0], stringLength))
        return false;

    // trim
    ansiString.erase(
        std::find_if(ansiString.begin(), ansiString.end(), [](const char& c)->bool {
            return c == 0;
        }), 
        ansiString.end());

    return true;
}

bool DK2ScenarioReader::ReadTimestamp()
{
    unsigned short dkyear;
    unsigned char dkday;
    unsigned char dkmonth;
    unsigned char dkhour;
    unsigned char dkminute;
    unsigned char dksecond;
    unsigned char dkbyte;
    unsigned short fillerBytes;

    READ_FROM_FSTREAM(mFileStream, dkyear);
    READ_FROM_FSTREAM(mFileStream, dkday);
    READ_FROM_FSTREAM(mFileStream, dkmonth);
    READ_FROM_FSTREAM(mFileStream, fillerBytes);
    READ_FROM_FSTREAM(mFileStream, dkhour);
    READ_FROM_FSTREAM(mFileStream, dkminute);
    READ_FROM_FSTREAM(mFileStream, dksecond);
    READ_FROM_FSTREAM(mFileStream, dkbyte);

    return true;
}

bool DK2ScenarioReader::Read32bitsFloat(float& outputFloat)
{
    unsigned int encoded_float;
    READ_FSTREAM_U32(mFileStream, encoded_float);

    // decode float
    outputFloat = encoded_float / DIVIDER_FLOAT;
    return true;
}

bool DK2ScenarioReader::ReadVector3f(glm::vec3& outputVector)
{
    if (!Read32bitsFloat(outputVector.x) ||
        !Read32bitsFloat(outputVector.y) ||
        !Read32bitsFloat(outputVector.z))
    {
        return false;
    }
    return true;
}

bool DK2ScenarioReader::ReadLight()
{
    unsigned int ikpos;
    READ_FSTREAM_U32(mFileStream, ikpos); // x, / ConversionUtils.FLOAT
    READ_FSTREAM_U32(mFileStream, ikpos); // y, / ConversionUtils.FLOAT
    READ_FSTREAM_U32(mFileStream, ikpos); // z, / ConversionUtils.FLOAT

    unsigned int iradius;
    READ_FSTREAM_U32(mFileStream, iradius); // / ConversionUtils.FLOAT

    unsigned int flags;
    READ_FSTREAM_U32(mFileStream, flags);

    unsigned char rgb;
    READ_FSTREAM_U8(mFileStream, rgb); // r
    READ_FSTREAM_U8(mFileStream, rgb); // g
    READ_FSTREAM_U8(mFileStream, rgb); // b
    READ_FSTREAM_U8(mFileStream, rgb); // a

    return true;
}

bool DK2ScenarioReader::ReadArtResource(ArtResourceDefinition& artResource)
{
    if (!ReadString8(64, artResource.mResourceName))
        return false;

    unsigned int resourceFlags = 0;
    READ_FSTREAM_U32(mFileStream, resourceFlags);

    // reading flags
    artResource.mPlayerColoured = (resourceFlags & ARTRESF_PLAYER_COLOURED) > 0;
    artResource.mAnimatingTexture = (resourceFlags & ARTRESF_ANIMATING_TEXTURE) > 0;
    artResource.mHasStartAnimation = (resourceFlags & ARTRESF_HAS_START_ANIMATION) > 0;
    artResource.mHasEndAnimation = (resourceFlags &  ARTRESF_HAS_END_ANIMATION) > 0;
    artResource.mRandomStartFrame = (resourceFlags & ARTRESF_RANDOM_START_FRAME) > 0;
    artResource.mOriginAtBottom = (resourceFlags & ARTRESF_ORIGIN_AT_BOTTOM) > 0;
    artResource.mDoesntLoop = (resourceFlags & ARTRESF_DOESNT_LOOP) > 0;
    artResource.mFlat = (resourceFlags & ARTRESF_FLAT) > 0;
    artResource.mDoesntUseProgressiveMesh = (resourceFlags & ARTRESF_DOESNT_USE_PROGRESSIVE_MESH) > 0;
    artResource.mUseAnimatingTextureForSelection = (resourceFlags & ARTRESF_USE_ANIMATING_TEXTURE_FOR_SELECTION) > 0;
    artResource.mPreload = (resourceFlags & ARTRESF_PRELOAD) > 0;
    artResource.mBlood = (resourceFlags & ARTRESF_BLOOD) > 0;

    union packed_data_t
    {
        struct
        {
            unsigned int mEncodedWidth;
            unsigned int mEncodedHeight;
            unsigned short mFrames;
            unsigned short mNone_1;
        } mImageType;
        struct
        {
            unsigned int mEncodedScale;
            unsigned short mFrames;
            unsigned short mNone_1;
            unsigned int mNone_2;
        } mMeshType;
        struct
        {
            unsigned int mFrames;
            unsigned int mFps;
            unsigned short mStartDist;
            unsigned short mEndDist;
        } mAnimType;
        struct
        {
            unsigned int mId;
            unsigned int mNone_1;
            unsigned int mNone_2;
        } mProcType;
        struct
        {
            unsigned int mNone_1;
            unsigned int mNone_2;
            unsigned int mNone_3;
        } mTerrainType;
        unsigned char mBytes[12];
    };
    packed_data_t packed_data;
    static_assert(sizeof(packed_data) == 12, "Wrong size");
    if (!mFileStream.read(reinterpret_cast<char*>(&packed_data), sizeof(packed_data)))
        return false;

    // map resource type
    unsigned char resourceType;
    READ_FSTREAM_U8(mFileStream, resourceType);
    if (!KWDParseENUM(resourceType, artResource.mResourceType))
        return false;

    if (artResource.mResourceType == eArtResource_AnimatingMesh)
    {
        READ_FSTREAM_U8(mFileStream, artResource.mStartAF);
        READ_FSTREAM_U8(mFileStream, artResource.mEndAF);
    }
    else
    {
        unsigned short unknownWord;
        READ_FSTREAM_U16(mFileStream, unknownWord);
    }

    unsigned char unknownByte;
    READ_FROM_FSTREAM(mFileStream, unknownByte);

    switch (artResource.mResourceType)
    {
        case eArtResource_Sprite:
        case eArtResource_Alpha:
        case eArtResource_AdditiveAlpha:
            artResource.mImageDesc.mFrames = packed_data.mImageType.mFrames;
            artResource.mImageDesc.mWidth = packed_data.mImageType.mEncodedWidth / DIVIDER_FLOAT;
            artResource.mImageDesc.mHeight = packed_data.mImageType.mEncodedHeight / DIVIDER_FLOAT;
        break;

        case eArtResource_Mesh:
        case eArtResource_MeshCollection:
            artResource.mMeshDesc.mFrames = packed_data.mMeshType.mFrames;
            artResource.mMeshDesc.mScale = packed_data.mMeshType.mEncodedScale / DIVIDER_FLOAT;
        break;

        case eArtResource_AnimatingMesh:
            artResource.mAnimationDesc.mFps = packed_data.mAnimType.mFps;
            artResource.mAnimationDesc.mFrames = packed_data.mAnimType.mFrames;
            artResource.mAnimationDesc.mDistStart = packed_data.mAnimType.mStartDist;
            artResource.mAnimationDesc.mDistEnd = packed_data.mAnimType.mEndDist;
        break;

        case eArtResource_ProceduralMesh:
            artResource.mProcDesc.mId = packed_data.mProcType.mId;
        break;
    }

    return true;
}

bool DK2ScenarioReader::ReadTerrainFlags(TerrainDefinition& terrainDef)
{
    unsigned int terrainFlags = 0;
    READ_FSTREAM_U32(mFileStream, terrainFlags);

    terrainDef.mIsSolid = (terrainFlags & TERRAINF_SOLID) > 0;
    terrainDef.mIsImpenetrable = (terrainFlags & TERRAINF_IMPENETRABLE) > 0;
    terrainDef.mIsOwnable = (terrainFlags & TERRAINF_OWNABLE) > 0;
    terrainDef.mIsTaggable = (terrainFlags & TERRAINF_TAGGABLE) > 0;
    terrainDef.mIsAttackable = (terrainFlags & TERRAINF_ATTACKABLE) > 0;
    terrainDef.mHasTorch = (terrainFlags & TERRAINF_TORCH) > 0;
    terrainDef.mIsWater = (terrainFlags & TERRAINF_WATER) > 0;
    terrainDef.mIsLava = (terrainFlags & TERRAINF_LAVA) > 0;
    terrainDef.mAlwaysExplored = (terrainFlags & TERRAINF_ALWAYS_EXPLORED) > 0;
    terrainDef.mPlayerColouredPath = (terrainFlags & TERRAINF_PLAYER_COLOURED_PATH) > 0;
    terrainDef.mPlayerColouredWall = (terrainFlags & TERRAINF_PLAYER_COLOURED_WALL) > 0;
    terrainDef.mConstructionTypeWater = (terrainFlags & TERRAINF_CONSTRUCTION_TYPE_WATER) > 0;
    terrainDef.mConstructionTypeQuad = (terrainFlags & TERRAINF_CONSTRUCTION_TYPE_QUAD) > 0;
    terrainDef.mUnexploreIfDugByAnotherPlayer = (terrainFlags & TERRAINF_UNEXPLORE_IF_DUG_BY_ANOTHER_PLAYER) > 0;
    terrainDef.mFillInable = (terrainFlags & TERRAINF_FILL_INABLE) > 0;
    terrainDef.mAllowRoomWalls = (terrainFlags & TERRAINF_ALLOW_ROOM_WALLS) > 0;
    terrainDef.mIsDecay = (terrainFlags & TERRAINF_DECAY) > 0;
    terrainDef.mHasRandomTexture = (terrainFlags & TERRAINF_RANDOM_TEXTURE) > 0;
    terrainDef.mTerrainColorR = (terrainFlags & TERRAINF_TERRAIN_COLOR_R) > 0;
    terrainDef.mTerrainColorG = (terrainFlags & TERRAINF_TERRAIN_COLOR_G) > 0;
    terrainDef.mTerrainColorB = (terrainFlags & TERRAINF_TERRAIN_COLOR_B) > 0;
    terrainDef.mDwarfCanDigThrough = (terrainFlags & TERRAINF_DWARF_CAN_DIG_THROUGH) > 0;
    terrainDef.mRevealThroughFogOfWar = (terrainFlags & TERRAINF_REVEAL_THROUGH_FOG_OF_WAR) > 0;
    terrainDef.mAmbientColorR = (terrainFlags & TERRAINF_AMBIENT_COLOR_R) > 0;
    terrainDef.mAmbientColorG = (terrainFlags & TERRAINF_AMBIENT_COLOR_G) > 0;
    terrainDef.mAmbientColorB = (terrainFlags & TERRAINF_AMBIENT_COLOR_B) > 0;
    terrainDef.mHasLight = (terrainFlags & TERRAINF_LIGHT) > 0;
    terrainDef.mHasAmbientLight = (terrainFlags & TERRAINF_AMBIENT_LIGHT) > 0;

    return true;
}

bool DK2ScenarioReader::ReadRoomFlags(RoomDefinition& roomDef)
{
    unsigned int roomFlags = 0;
    READ_FSTREAM_U32(mFileStream, roomFlags);

    roomDef.mPlaceableOnWater = (roomFlags & ROOMF_PLACEABLE_ON_WATER) > 0;
    roomDef.mPlaceableOnLava = (roomFlags & ROOMF_PLACEABLE_ON_LAVA) > 0;
    roomDef.mPlaceableOnLand = (roomFlags & ROOMF_PLACEABLE_ON_LAND) > 0;
    roomDef.mHasWalls = (roomFlags & ROOMF_HAS_WALLS) > 0;
    roomDef.mCentre = (roomFlags & ROOMF_CENTRE) > 0;
    roomDef.mSpecialTiles = (roomFlags & ROOMF_SPECIAL_TILES) > 0;
    roomDef.mNormalTiles = (roomFlags & ROOMF_NORMAL_TILES) > 0;
    roomDef.mBuildable = (roomFlags & ROOMF_BUILDABLE) > 0;
    roomDef.mSpecialWalls = (roomFlags & ROOMF_SPECIAL_WALLS) > 0;
    roomDef.mIsAttackable = (roomFlags & ROOMF_ATTACKABLE) > 0;
    roomDef.mHasFlame = (roomFlags & ROOMF_HAS_FLAME) > 0;
    roomDef.mIsGood = (roomFlags & ROOMF_IS_GOOD) > 0;

    return true;
}

bool DK2ScenarioReader::ReadObjectFlags(GameObjectDefinition& objectDef)
{
    unsigned int objectFlags;
    READ_FSTREAM_U32(mFileStream, objectFlags);

    objectDef.mDieOverTime = (objectFlags & OBJECTF_DIE_OVER_TIME) > 0;
    objectDef.mDieOverTimeIfNotInRoom = (objectFlags & OBJECTF_DIE_OVER_TIME_IF_NOT_IN_ROOM) > 0;
    objectDef.mCanBePickedUp = (objectFlags & OBJECTF_CAN_BE_PICKED_UP) > 0;
    objectDef.mCanBeSlapped = (objectFlags & OBJECTF_CAN_BE_SLAPPED) > 0;
    objectDef.mDieWhenSlapped = (objectFlags & OBJECTF_DIE_WHEN_SLAPPED) > 0;
    objectDef.mCanBeDroppedOnAnyLand = (objectFlags & OBJECTF_CAN_BE_DROPPED_ON_ANY_LAND) > 0;
    objectDef.mObstacle = (objectFlags & OBJECTF_OBSTACLE) > 0;
    objectDef.mBounce = (objectFlags & OBJECTF_BOUNCE) > 0;
    objectDef.mBoulderCanRollThrough = (objectFlags & OBJECTF_BOULDER_CAN_ROLL_THROUGH) > 0;
    objectDef.mBoulderDestroys = (objectFlags & OBJECTF_BOULDER_DESTROYS) > 0;
    objectDef.mIsPillar = (objectFlags & OBJECTF_PILLAR) > 0;
    objectDef.mDoorKey = (objectFlags & OBJECTF_DOOR_KEY) > 0;
    objectDef.mIsDamageable = (objectFlags & OBJECTF_DAMAGEABLE) > 0;
    objectDef.mHighlightable = (objectFlags & OBJECTF_HIGHLIGHTABLE) > 0;
    objectDef.mPlaceable = (objectFlags & OBJECTF_PLACEABLE) > 0;
    objectDef.mFirstPersonObstacle = (objectFlags & OBJECTF_FIRST_PERSON_OBSTACLE) > 0;
    objectDef.mSolidObstacle = (objectFlags & OBJECTF_SOLID_OBSTACLE) > 0;
    objectDef.mCastShadows = (objectFlags & OBJECTF_CAST_SHADOWS) > 0;

    // category
    objectDef.mObjectCategory = eGameObjectCategory_Normal;
    {
        const unsigned int categoryBitsMask = OBJECTF_TYPE_SPECIAL | OBJECTF_TYPE_SPELL_BOOK | OBJECTF_TYPE_CRATE | 
            OBJECTF_TYPE_LAIR | OBJECTF_TYPE_GOLD | OBJECTF_TYPE_FOOD | OBJECTF_TYPE_LEVEL_GEM;
        switch (objectFlags & categoryBitsMask)
        {
            case OBJECTF_TYPE_SPECIAL   : objectDef.mObjectCategory = eGameObjectCategory_Special; break;
            case OBJECTF_TYPE_SPELL_BOOK: objectDef.mObjectCategory = eGameObjectCategory_SpellBook; break;
            case OBJECTF_TYPE_CRATE     : objectDef.mObjectCategory = eGameObjectCategory_Crate; break;
            case OBJECTF_TYPE_LAIR      : objectDef.mObjectCategory = eGameObjectCategory_Lair; break;
            case OBJECTF_TYPE_GOLD      : objectDef.mObjectCategory = eGameObjectCategory_Gold; break;
            case OBJECTF_TYPE_FOOD      : objectDef.mObjectCategory = eGameObjectCategory_Food; break;
            case OBJECTF_TYPE_LEVEL_GEM : objectDef.mObjectCategory = eGameObjectCategory_LevelGem; break;
            default:
                cxx_assert((objectFlags & categoryBitsMask) == 0);
            break;
        }
    }

    return true;
}

bool DK2ScenarioReader::ReadStringId()
{
    unsigned int ids[5];
    for (unsigned int& idEntry : ids)
    {
        READ_FROM_FSTREAM(mFileStream, idEntry);
    }

    unsigned int unknownDword;
    READ_FROM_FSTREAM(mFileStream, unknownDword);

    return true;
}

bool DK2ScenarioReader::ReadMapData(ScenarioDefinition& scenarioData)
{
    scenarioData.mMapTiles.resize(scenarioData.mLevelDimensionX * scenarioData.mLevelDimensionY);

    // read tiles
    for (int tiley = 0; tiley < scenarioData.mLevelDimensionY; ++tiley)
    for (int tilex = 0; tilex < scenarioData.mLevelDimensionX; ++tilex)
    {
        const int tileIndex = (tiley * scenarioData.mLevelDimensionX) + tilex;

        // terrain type is not mapped to internal id so it can be red as is
        READ_FSTREAM_U8(mFileStream, scenarioData.mMapTiles[tileIndex].mTerrainType);

        unsigned char playerID;
        READ_FSTREAM_U8(mFileStream, playerID);
        if (!KWDParseENUM(playerID, scenarioData.mMapTiles[tileIndex].mOwnerID))
            return false;

        unsigned char bridgeTerrain;
        READ_FSTREAM_U8(mFileStream, bridgeTerrain);
        if (!KWDParseENUM(bridgeTerrain, scenarioData.mMapTiles[tileIndex].mTerrainUnderTheBridge))
            return false;

        unsigned char filler;
        READ_FROM_FSTREAM(mFileStream, filler);
    }

    return true;
}

bool DK2ScenarioReader::ReadScenarioVariables(int numElements, ScenarioDefinition& scenarioData)
{
    for (int ielement = 0; ielement < numElements; ++ielement)
    {
        int variableType = 0;
        READ_FROM_FSTREAM(mFileStream, variableType);

        if (variableType == ScenarioVariableType_CreaturePool)
        {
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // creature id
            READ_FROM_FSTREAM(mFileStream, dummyInt); // value
            READ_FROM_FSTREAM(mFileStream, dummyInt); // player id
            continue;
        }

        if (variableType == ScenarioVariableType_Availability)
        {
            unsigned short dummyWord;
            READ_FROM_FSTREAM(mFileStream, dummyWord); // availability type
            READ_FROM_FSTREAM(mFileStream, dummyWord); // player id
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // type id
            READ_FROM_FSTREAM(mFileStream, dummyInt); // availability value
            continue;
        }

        if (variableType == ScenarioVariableType_SacrificesId)
        {
            unsigned char dummyByte;
            READ_FROM_FSTREAM(mFileStream, dummyByte); // type 1
            READ_FROM_FSTREAM(mFileStream, dummyByte); // id 1
            READ_FROM_FSTREAM(mFileStream, dummyByte); // type 2
            READ_FROM_FSTREAM(mFileStream, dummyByte); // id 2
            READ_FROM_FSTREAM(mFileStream, dummyByte); // type 3
            READ_FROM_FSTREAM(mFileStream, dummyByte); // id 3
            READ_FROM_FSTREAM(mFileStream, dummyByte); // sacrifice reward type
            READ_FROM_FSTREAM(mFileStream, dummyByte); // speech id
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // reward value
            continue;
        }

        if ((variableType == ScenarioVariableType_CreatureStatsId) ||
            (variableType == ScenarioVariableType_CreatureFirstPersonId))
        {
            int dummyInt;
            READ_FROM_FSTREAM(mFileStream, dummyInt); // stat type
            READ_FROM_FSTREAM(mFileStream, dummyInt); // value
            READ_FROM_FSTREAM(mFileStream, dummyInt); // level
            continue;
        }

        // common
        int intValue;
        READ_FROM_FSTREAM(mFileStream, intValue); // value

        int dummyInt;
        READ_FROM_FSTREAM(mFileStream, dummyInt); // unknown 1
        READ_FROM_FSTREAM(mFileStream, dummyInt); // unknown 1

        switch (variableType)
        {
            case ScenarioVariableType_GoldMinedFromGems:
                scenarioData.mVariables.mGoldMinedFromGems = intValue;
            break;
            case ScenarioVariableType_MaxGoldPerTreasuryTile:
                scenarioData.mVariables.mMaxGoldPerTreasuryTile = intValue;
            break;
            case ScenarioVariableType_SpecialIncreaseGoldAmount:
                scenarioData.mVariables.mSpecialIncreaseGoldAmount = intValue;
            break;
            case ScenarioVariableType_MaxGoldPileOutsideTreasury:
                scenarioData.mVariables.mMaxGoldPileOutsideTreasury = intValue;
            break;
            case ScenarioVariableType_MaxGoldPerDungeonHeartTile:
                scenarioData.mVariables.mMaxGoldPerDungeonHeartTile = intValue;
            break;
            case ScenarioVariableType_MaximumManaThreshold:
                scenarioData.mVariables.mMaximumManaThreshold = intValue;
            break;
        }
    }

    return true;
}

bool DK2ScenarioReader::ReadObjectDefinition(GameObjectDefinition& objectDef)
{
    if (!ReadString8(32, objectDef.mObjectName))
        return false;

    // resources
    if (!ReadArtResource(objectDef.mResourceMesh))
        return false;

    if (!ReadArtResource(objectDef.mResourceGuiIcon))
        return false;

    if (!ReadArtResource(objectDef.mResourceInHandIcon))
        return false;

    if (!ReadArtResource(objectDef.mResourceInHandMesh))
        return false;

    if (!ReadArtResource(objectDef.mResourceUnknown))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional1))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional2))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional3))
        return false;

    if (!ReadArtResource(objectDef.mResourceAdditional4))
        return false;

    if (!ReadLight())
        return false;

    unsigned int width;
    unsigned int height;
    unsigned int mass;
    unsigned int speed;
    unsigned int airFriction;

    READ_FROM_FSTREAM(mFileStream, width);
    READ_FROM_FSTREAM(mFileStream, height);
    READ_FROM_FSTREAM(mFileStream, mass); 
    READ_FROM_FSTREAM(mFileStream, speed);
    READ_FROM_FSTREAM(mFileStream, airFriction);

    // set params
    objectDef.mWidth = (width * 1.0f) / DIVIDER_FLOAT;
    objectDef.mHeight = (height * 1.0f) / DIVIDER_FLOAT;
    objectDef.mMass = (mass * 1.0f) / DIVIDER_FLOAT;
    objectDef.mSpeed = (speed * 1.0f) / DIVIDER_FLOAT;
    objectDef.mAirFriction = (airFriction * 1.0f) / DIVIDER_DOUBLE;

    unsigned char objMaterial;
    READ_FSTREAM_U8(mFileStream, objMaterial);
    if (!KWDParseENUM(objMaterial, objectDef.mObjectMaterial))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 3);

    if (!ReadObjectFlags(objectDef))
        return false;

    READ_FSTREAM_U16(mFileStream, objectDef.mHitpoints);
    READ_FSTREAM_U16(mFileStream, objectDef.mMaxAngle);
    SKIP_FSTREAM_BYTES(mFileStream, 2);
    READ_FSTREAM_U16(mFileStream, objectDef.mManaValue);
    READ_FSTREAM_U16(mFileStream, objectDef.mTooltipStringId);
    READ_FSTREAM_U16(mFileStream, objectDef.mNameStringId);
    READ_FSTREAM_U16(mFileStream, objectDef.mSlapEffectId);
    READ_FSTREAM_U16(mFileStream, objectDef.mDeathEffectId);
    READ_FSTREAM_U16(mFileStream, objectDef.mMiscEffectId);

    // object type is not mapped to internal id so it can be red as is
    READ_FSTREAM_U8(mFileStream, objectDef.mObjectClass);

    unsigned char initialState;
    READ_FROM_FSTREAM(mFileStream, initialState);

    cxx_assert(initialState < eGameObjectState_MAX);
    objectDef.mStartState = static_cast<eGameObjectState>(initialState);

    READ_FSTREAM_U8(mFileStream, objectDef.mRoomCapacity);

    unsigned char pickupPriority;
    READ_FROM_FSTREAM(mFileStream, pickupPriority);

    // sound category
    if (!ReadString8(32, objectDef.mSoundCategory))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadObjectsData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mGameObjectDefs.resize(numElements + 1);
    scenarioData.mGameObjectDefs[0] = {}; // dummy element

    // read definitions
    for (int iobject = 1; iobject < numElements + 1; ++iobject)
    {
        if (!ReadObjectDefinition(scenarioData.mGameObjectDefs[iobject]))
            return false;

        bool correctId = (scenarioData.mGameObjectDefs[iobject].mObjectClass == iobject);
        cxx_assert(correctId);
    }
    return true;
}

bool DK2ScenarioReader::ReadPlayerDefinition(PlayerDefinition& playerDef)
{
    READ_FSTREAM_U32(mFileStream, playerDef.mInitialGold);

    unsigned int playerType;
    READ_FSTREAM_U32(mFileStream, playerType);
    if (!KWDParseENUM(playerType, playerDef.mPlayerType))
        return false;

    unsigned char aiType;
    READ_FSTREAM_U8(mFileStream, aiType);
    if (!KWDParseENUM(aiType, playerDef.mComputerAI))
        return false;

    unsigned char speed;
    READ_FROM_FSTREAM(mFileStream, speed);

    unsigned char openness;
    READ_FROM_FSTREAM(mFileStream, openness);

    unsigned char fillerByte;
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    unsigned int fillerDword;
    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerByte);

    // build order
    unsigned char buildOrderBytes[15];
    if (!mFileStream.read(reinterpret_cast<char*>(buildOrderBytes), sizeof(buildOrderBytes)))
        return false;

    READ_FROM_FSTREAM(mFileStream, fillerByte); // flexibility
    READ_FROM_FSTREAM(mFileStream, fillerByte); // digToNeutralRoomsWithinTilesOfClaimedArea
    
    unsigned short fillerWord;
    READ_FROM_FSTREAM(mFileStream, fillerWord); // removeCallToArmsAfterSeconds

    READ_FROM_FSTREAM(mFileStream, fillerDword); // boulderTrapsOnLongCorridors
    READ_FROM_FSTREAM(mFileStream, fillerDword); // boulderTrapsOnRouteToBreachPoints

    READ_FROM_FSTREAM(mFileStream, fillerByte); // trapUseStyle
    READ_FROM_FSTREAM(mFileStream, fillerByte); // doorTrapPreference

    READ_FROM_FSTREAM(mFileStream, fillerByte); // doorUsage
    READ_FROM_FSTREAM(mFileStream, fillerByte); // chanceOfLookingToUseTrapsAndDoors

    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireMinLevelForCreatures
    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireTotalThreatGreaterThanTheEnemy
    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireAllRoomTypesPlaced
    READ_FROM_FSTREAM(mFileStream, fillerDword); // requireAllKeeperSpellsResearched
    READ_FROM_FSTREAM(mFileStream, fillerDword); // onlyAttackAttackers
    READ_FROM_FSTREAM(mFileStream, fillerDword); // neverAttack

    READ_FROM_FSTREAM(mFileStream, fillerByte); // minLevelForCreatures
    READ_FROM_FSTREAM(mFileStream, fillerByte); // totalThreatGreaterThanTheEnemy
    READ_FROM_FSTREAM(mFileStream, fillerByte); // firstAttemptToBreachRoom
    READ_FROM_FSTREAM(mFileStream, fillerByte); // firstDigToEnemyPoint
    READ_FROM_FSTREAM(mFileStream, fillerByte); // breachAtPointsSimultaneously
    READ_FROM_FSTREAM(mFileStream, fillerByte); // usePercentageOfTotalCreaturesInFirstFightAfterBreach

    READ_FSTREAM_U16(mFileStream, playerDef.mManaValue); 
    READ_FROM_FSTREAM(mFileStream, fillerWord); // placeCallToArmsWhereThreatValueIsGreaterThan
    READ_FROM_FSTREAM(mFileStream, fillerWord); // removeCallToArmsIfLessThanEnemyCreatures

    READ_FROM_FSTREAM(mFileStream, fillerWord); // removeCallToArmsIfLessThanEnemyCreaturesWithinTiles
    READ_FROM_FSTREAM(mFileStream, fillerWord); // pullCreaturesFromFightIfOutnumberedAndUnableToDropReinforcements

    READ_FROM_FSTREAM(mFileStream, fillerByte); // threatValueOfDroppedCreaturesIsPercentageOfEnemyThreatValue
    READ_FROM_FSTREAM(mFileStream, fillerByte); // spellStyle
    READ_FROM_FSTREAM(mFileStream, fillerByte); // attemptToImprisonPercentageOfEnemyCreatures
    READ_FROM_FSTREAM(mFileStream, fillerByte); // ifCreatureHealthIsPercentageAndNotInOwnRoomMoveToLairOrTemple

    READ_FSTREAM_U16(mFileStream, playerDef.mGoldValue);

    READ_FROM_FSTREAM(mFileStream, fillerDword); // tryToMakeUnhappyOnesHappy
    READ_FROM_FSTREAM(mFileStream, fillerDword); // tryToMakeAngryOnesHappy
    READ_FROM_FSTREAM(mFileStream, fillerDword); // disposeOfAngryCreatures
    READ_FROM_FSTREAM(mFileStream, fillerDword); // disposeOfRubbishCreaturesIfBetterOnesComeAlong

    READ_FROM_FSTREAM(mFileStream, fillerByte); // disposalMethod
    READ_FROM_FSTREAM(mFileStream, fillerByte); // maximumNumberOfImps
    READ_FROM_FSTREAM(mFileStream, fillerByte); // willNotSlapCreatures
    READ_FROM_FSTREAM(mFileStream, fillerByte); // attackWhenNumberOfCreaturesIsAtLeast

    READ_FROM_FSTREAM(mFileStream, fillerDword); // useLightningIfEnemyIsInWater

    READ_FROM_FSTREAM(mFileStream, fillerByte); // useSightOfEvil
    READ_FROM_FSTREAM(mFileStream, fillerByte); // useSpellsInBattle
    READ_FROM_FSTREAM(mFileStream, fillerByte); // spellsPowerPreference
    READ_FROM_FSTREAM(mFileStream, fillerByte); // useCallToArms
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerWord); // mineGoldUntil
    READ_FROM_FSTREAM(mFileStream, fillerWord); // waitSecondsAfterPreviousAttackBeforeAttackingAgain

    READ_FSTREAM_U32(mFileStream, playerDef.mInitialMana);

    READ_FROM_FSTREAM(mFileStream, fillerWord); // exploreUpToTilesToFindSpecials
    READ_FROM_FSTREAM(mFileStream, fillerWord); // impTilesRatio
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildStartX
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildStartY
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildEndX
    READ_FROM_FSTREAM(mFileStream, fillerWord); // buildEndY

    READ_FROM_FSTREAM(mFileStream, fillerByte); // likelyhoodToMovingCreaturesToLibraryForResearching
    READ_FROM_FSTREAM(mFileStream, fillerByte); // chanceOfExploringToFindSpecials
    READ_FROM_FSTREAM(mFileStream, fillerByte); // chanceOfFindingSpecialsWhenExploring
    READ_FROM_FSTREAM(mFileStream, fillerByte); // fateOfImprisonedCreatures

    READ_FROM_FSTREAM(mFileStream, fillerWord); // triggerId

    unsigned char playerID;
    READ_FSTREAM_U8(mFileStream, playerID);
    if (!KWDParseENUM(playerID, playerDef.mPlayerId))
        return false;

    READ_FSTREAM_U16(mFileStream, playerDef.mStartCameraX); // cameraX
    READ_FSTREAM_U16(mFileStream, playerDef.mStartCameraY); // cameraY

    if (!ReadString8(32, playerDef.mPlayerName))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadCreatureThingBehaviorFlags(ScenarioCreatureThing& creatureThingData)
{
    unsigned char flags = 0;
    READ_FSTREAM_U8(mFileStream, flags);

    creatureThingData.mBehaviorFlags.mWillFight = CHECK_BIT_ON(flags, 0);
    creatureThingData.mBehaviorFlags.mIsLeader = CHECK_BIT_ON(flags, 1);
    creatureThingData.mBehaviorFlags.mIsFollower = CHECK_BIT_ON(flags, 2);
    creatureThingData.mBehaviorFlags.mWillBeAttacked = CHECK_BIT_ON(flags, 3);
    creatureThingData.mBehaviorFlags.mReturnToHeroLair = CHECK_BIT_ON(flags, 4);
    creatureThingData.mBehaviorFlags.mFreeFriendsOnJailBreak = CHECK_BIT_ON(flags, 5);
    creatureThingData.mBehaviorFlags.mActAsDropped = CHECK_BIT_ON(flags, 6);
    creatureThingData.mBehaviorFlags.mStartAsDying = CHECK_BIT_ON(flags, 7);

    return true;
}

bool DK2ScenarioReader::ReadCreatureThingData(ScenarioCreatureThing& creatureThingData)
{
    READ_FSTREAM_I32(mFileStream, creatureThingData.mPositionX);
    READ_FSTREAM_I32(mFileStream, creatureThingData.mPositionY);
    READ_FSTREAM_I32(mFileStream, creatureThingData.mPositionZ);
    READ_FSTREAM_U16(mFileStream, creatureThingData.mGoldHeld);
    READ_FSTREAM_U8(mFileStream, creatureThingData.mLevel);

    if (!ReadCreatureThingBehaviorFlags(creatureThingData))
        return false;

    READ_FSTREAM_I32(mFileStream, creatureThingData.mInitialHealth);
    
    int objectiveTargetPlayerId;
    READ_FSTREAM_I32(mFileStream, objectiveTargetPlayerId);
    if (!KWDParseENUM(objectiveTargetPlayerId, creatureThingData.mObjectiveTargetPlayerId))
        return false;

    SKIP_FSTREAM_U16(mFileStream); // trigger id 

    READ_FSTREAM_U8(mFileStream, creatureThingData.mCreatureTypeId);

    unsigned char playerId;
    READ_FSTREAM_U8(mFileStream, playerId);
    if (!KWDParseENUM(playerId, creatureThingData.mPlayerId))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadObjectThingData(ScenarioObjectThing& objectThingData)
{
    READ_FSTREAM_DATATYPE(mFileStream, objectThingData.mPositionX, int);
    READ_FSTREAM_DATATYPE(mFileStream, objectThingData.mPositionY, int);

    unsigned int unknownData;
    READ_FROM_FSTREAM(mFileStream, unknownData);

    int keeperSpellId = 0;
    READ_FROM_FSTREAM(mFileStream, keeperSpellId);
    objectThingData.mKeeperSpellId = keeperSpellId;

    int moneyAmount = 0;
    READ_FROM_FSTREAM(mFileStream, moneyAmount);
    objectThingData.mMoneyAmount = moneyAmount;

    unsigned short triggerId = 0;
    READ_FROM_FSTREAM(mFileStream, triggerId);
    objectThingData.mTriggerId = triggerId;

    unsigned char objectId = 0;
    READ_FROM_FSTREAM(mFileStream, objectId);
    objectThingData.mObjectClassId = objectId;

    unsigned char playerId = 0;
    READ_FROM_FSTREAM(mFileStream, playerId);
    if (!KWDParseENUM(playerId, objectThingData.mPlayerId))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadRoomThingData(ScenarioRoomThing& roomThingData)
{
    READ_FSTREAM_DATATYPE(mFileStream, roomThingData.mPositionX, int);
    READ_FSTREAM_DATATYPE(mFileStream, roomThingData.mPositionY, int);

    int unknownInt;
    READ_FROM_FSTREAM(mFileStream, unknownInt);

    short unknownShort;
    READ_FROM_FSTREAM(mFileStream, unknownShort);

    unsigned char direction;
    READ_FROM_FSTREAM(mFileStream, direction);

    if (!KWDParseENUM(direction, roomThingData.mDirection))
    {
        cxx_assert(false);
        return false;
    }
    unsigned char unknownByte;
    READ_FROM_FSTREAM(mFileStream, unknownByte);

    READ_FSTREAM_DATATYPE(mFileStream, roomThingData.mInitialHealth, unsigned short);

    unsigned char roomType = 0;
    READ_FROM_FSTREAM(mFileStream, roomType);

    if (!KWDRoomThingToRoomType(roomType, roomThingData.mRoomType))
    {
        cxx_assert(false);
        return false;
    }

    unsigned char playerId = 0;
    READ_FROM_FSTREAM(mFileStream, playerId);
    if (!KWDParseENUM(playerId, roomThingData.mPlayerId))
    {
        cxx_assert(false);
        return false;
    }
    return true;
}

bool DK2ScenarioReader::ReadThingsData(int numElements, ScenarioDefinition& scenarioData)
{
    for (int ielement = 0; ielement < numElements; ++ielement)
    {
        unsigned int thingType = 0;
        READ_FROM_FSTREAM(mFileStream, thingType);
        unsigned int dataSize = 0;
        READ_FROM_FSTREAM(mFileStream, dataSize);

        bool wasSkipped = true;
        switch (thingType)
        {
            case DK_OBJECT_THING:
            {
                wasSkipped = false;
                if (!ReadObjectThingData(scenarioData.mObjectThings.emplace_back()))
                {
                    scenarioData.mObjectThings.pop_back();
                    return false;
                }
            }
            break;
            case DK_TRAP_THING:
            break;
            case DK_DOOR_THING:
            break;
            case DK_ACTIONPOINT_THING:
            break;
            case DK_NEUTRAL_CREATURE_THING:
            break;
            case DK_GOOD_CREATURE_THING:
            break;
            case DK_CREATURE_THING:
            {
                wasSkipped = false;
                if (!ReadCreatureThingData(scenarioData.mCreatureThings.emplace_back()))
                {
                    scenarioData.mCreatureThings.pop_back();
                    return false;
                }
            }
            break;
            case DK_HEROPARTY_THING:
            break;
            case DK_DEAD_BODY_THING:
            break;
            case DK_EFFECT_GENERATOR_THING:
            break;
            case DK_ROOM_THING:
            {
                wasSkipped = false;
                if (!ReadRoomThingData(scenarioData.mRoomThings.emplace_back()))
                {
                    scenarioData.mRoomThings.pop_back();
                    return false;
                }
            }
            break;
            case DK_CAMERA_THING:
            break;
            default:
                cxx_assert(false);
            break;
        }

        if (wasSkipped)
        {
            SKIP_FSTREAM_BYTES(mFileStream, dataSize);
        }
    }
    return true;
}

bool DK2ScenarioReader::ReadPlayersData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mPlayerDefs.resize(numElements + 1);
    scenarioData.mPlayerDefs[0] = {}; // dummy element

    // read definitions
    for (int iplayer = 1; iplayer < numElements + 1; ++iplayer)
    {
        if (!ReadPlayerDefinition(scenarioData.mPlayerDefs[iplayer]))
            return false;

        bool correctId = (scenarioData.mPlayerDefs[iplayer].mPlayerId == iplayer);
        cxx_assert(correctId);
    }
    return true;
}

bool DK2ScenarioReader::ReadRoomDefinition(RoomDefinition& roomDef)
{
    if (!ReadString8(32, roomDef.mRoomName))
        return false;

    // resources
    if (!ReadArtResource(roomDef.mGuiIcon))
        return false;

    if (!ReadArtResource(roomDef.mEditorIcon))
        return false;

    if (!ReadArtResource(roomDef.mCompleteResource))
        return false;

    if (!ReadArtResource(roomDef.mStraightResource))
        return false;

    if (!ReadArtResource(roomDef.mInsideCornerResource))
        return false;

    if (!ReadArtResource(roomDef.mUnknownResource))
        return false;

    if (!ReadArtResource(roomDef.mOutsideCornerResource))
        return false;

    if (!ReadArtResource(roomDef.mWallResource))
        return false;

    if (!ReadArtResource(roomDef.mCapResource))
        return false;

    if (!ReadArtResource(roomDef.mCeilingResource))
        return false;

    unsigned int ceilingHeight;
    READ_FROM_FSTREAM(mFileStream, ceilingHeight);

    unsigned short fillerWord;
    READ_FROM_FSTREAM(mFileStream, fillerWord);

    unsigned short torchIntensity;
    READ_FROM_FSTREAM(mFileStream, torchIntensity);

    if (!ReadRoomFlags(roomDef))
        return false;

    unsigned short tooltipStringId;
    READ_FROM_FSTREAM(mFileStream, tooltipStringId);

    unsigned short nameStringId;
    READ_FROM_FSTREAM(mFileStream, nameStringId);
    READ_FSTREAM_U16(mFileStream, roomDef.mCost);

    unsigned short fightEffectId;
    READ_FROM_FSTREAM(mFileStream, fightEffectId);

    unsigned short generalDescriptionStringId;
    READ_FROM_FSTREAM(mFileStream, generalDescriptionStringId);

    unsigned short strenghtStringId;
    READ_FROM_FSTREAM(mFileStream, strenghtStringId);

    unsigned short torchRadius;
    READ_FROM_FSTREAM(mFileStream, torchRadius);

    unsigned short effects[8];
    for (unsigned short& effectEntry : effects)
    {
        READ_FROM_FSTREAM(mFileStream, effectEntry);
    }
    READ_FSTREAM_U8(mFileStream, roomDef.mRoomType);

    unsigned char fillerByte;
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FSTREAM_U8(mFileStream, roomDef.mTerrainType);

    unsigned char tileConstruction;
    READ_FSTREAM_U8(mFileStream, tileConstruction);
    if (!KWDParseENUM(tileConstruction, roomDef.mTileConstruction))
        return false;

    unsigned char createdCreatureId;
    READ_FROM_FSTREAM(mFileStream, createdCreatureId);

    unsigned char torchColor[3];
    for (unsigned char& colorComponent : torchColor)
    {
        READ_FROM_FSTREAM(mFileStream, colorComponent);
    }

    for (GameObjectClassId& objectid : roomDef.mObjectIds)
    {
        READ_FSTREAM_U8(mFileStream, objectid);
    }

    if (!ReadString8(32, roomDef.mSoundCategory))
        return false;

    READ_FSTREAM_U8(mFileStream, roomDef.mOrderInEditor);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    if (!ReadArtResource(roomDef.mTorchResource))
        return false;

    READ_FSTREAM_U8(mFileStream, roomDef.mRecommendedSizeX);
    READ_FSTREAM_U8(mFileStream, roomDef.mRecommendedSizeY);

    short healthGain;
    READ_FROM_FSTREAM(mFileStream, healthGain);

    return true;
}

bool DK2ScenarioReader::ReadRoomsData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mRoomDefs.resize(numElements + 1);
    scenarioData.mRoomDefs[0] = {}; // dummy element

    // read definitions
    for (int iroom = 1; iroom < numElements + 1; ++iroom)
    {
        if (!ReadRoomDefinition(scenarioData.mRoomDefs[iroom]))
            return false;

        bool correctId = (scenarioData.mRoomDefs[iroom].mRoomType == iroom);
        cxx_assert(correctId);
    }

    return true;
}

bool DK2ScenarioReader::ReadCreaturesDefinition(CreatureDefinition& creature)
{
    if (!ReadString8(32, creature.mCreatureName))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 84); // unknown data
    
    // parse primary animations
    for (int ianim = 0; ianim < 36; ++ianim)
    {
        if (!ReadArtResource(creature.mAnimationResources[ianim]))
            return false;
    }

    if (!ReadArtResource(creature.mIcon1Resource))
        return false;

    if (!ReadArtResource(creature.mIcon2Resource))
        return false;

    SKIP_FSTREAM_U16(mFileStream);
    SKIP_FSTREAM_U32(mFileStream);
    SKIP_FSTREAM_U32(mFileStream);

    READ_FSTREAM_U8(mFileStream, creature.mOrderInEditor);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdGeneral);
    if (!Read32bitsFloat(creature.mShotDelay))
        return false;

    READ_FSTREAM_U16(mFileStream, creature.mOlhiEffectId);
    READ_FSTREAM_U16(mFileStream, creature.mIntroductionStringId);
    if (!Read32bitsFloat(creature.mPerceptionRange))
        return false;
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdLair);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdFood);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdPay);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdWork);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdSlap);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdHeld);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdLonely);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdHatred);
    READ_FSTREAM_U16(mFileStream, creature.mAngerStringIdTorture);
    if (!ReadString8(32, creature.mTranslationSoundGategory))
        return false;
    if (!Read32bitsFloat(creature.mShuffleSpeed))
        return false;
    READ_FSTREAM_U8(mFileStream, creature.mCloneCreatureTypeId);
    SKIP_FSTREAM_U8(mFileStream); // first person gamma effect

    unsigned char fpWalkCycleScale;
    READ_FSTREAM_U8(mFileStream, fpWalkCycleScale); // first person walk cycle scale, Movement
    SKIP_FSTREAM_U8(mFileStream); // intro camera path index
    SKIP_FSTREAM_U8(mFileStream);

    if (!ReadArtResource(creature.mPortraitResource))
        return false;

    if (!ReadLight()) return false;
    // max 2 attractions
    if (!ReadCreatureAttraction()) return false;
    if (!ReadCreatureAttraction()) return false;

    float firstPersonWaddleScale; // movement
    if (!Read32bitsFloat(firstPersonWaddleScale))
        return false;

    float firstPersonOscillateScale; // movement
    if (!Read32bitsFloat(firstPersonOscillateScale))
        return false;

    // max 3 spells
    if (!ReadCreatureSpell()) return false;
    if (!ReadCreatureSpell()) return false;
    if (!ReadCreatureSpell()) return false;

    // max 4 resistances
    if (!ReadCreatureResistance()) return false;
    if (!ReadCreatureResistance()) return false;
    if (!ReadCreatureResistance()) return false;
    if (!ReadCreatureResistance()) return false;

    // max 3 happy jobs
    if (!ReadCreatureJobPreference()) return false;
    if (!ReadCreatureJobPreference()) return false;
    if (!ReadCreatureJobPreference()) return false;

    // max 2 unhappy jobs
    if (!ReadCreatureJobPreference()) return false;
    if (!ReadCreatureJobPreference()) return false;

    // max 3 angry jobs
    if (!ReadCreatureJobPreference()) return false;
    if (!ReadCreatureJobPreference()) return false;
    if (!ReadCreatureJobPreference()) return false;

    eCreatureJob hateJobs[2];
    for (int i = 0; i < CountOf(hateJobs); ++i)
    {
        unsigned int jobType;
        READ_FROM_FSTREAM(mFileStream, jobType);
        if (!KWDParseENUM(jobType, hateJobs[i]))
        {
            cxx_assert(false);
        }
    }

    // max 3 alternative jobs
    if (!ReadCreatureJobAlternative()) return false;
    if (!ReadCreatureJobAlternative()) return false;
    if (!ReadCreatureJobAlternative()) return false;

    if (!ReadVector3f(creature.mAnimationOffset))
        return false;

    SKIP_FSTREAM_U32(mFileStream);

    if (!Read32bitsFloat(creature.mHeight))
        return false;

    SKIP_FSTREAM_U32(mFileStream); // float32 ?
    SKIP_FSTREAM_U32(mFileStream);
    if (!Read32bitsFloat(creature.mEyeHeight))
        return false;
    if (!Read32bitsFloat(creature.mSpeed))
        return false;
    if (!Read32bitsFloat(creature.mRunSpeed))
        return false;
    if (!Read32bitsFloat(creature.mHungerRate))
        return false;

    READ_FSTREAM_I32(mFileStream, creature.mTimeAwake);
    READ_FSTREAM_I32(mFileStream, creature.mTimeSleep);
    if (!Read32bitsFloat(creature.mDistanceCanSee))
        return false;
    if (!Read32bitsFloat(creature.mDistanceCanHear))
        return false;
    if (!Read32bitsFloat(creature.mStunDuration))
        return false;
    if (!Read32bitsFloat(creature.mGuardDuration))
        return false;
    if (!Read32bitsFloat(creature.mIdleDuration))
        return false;
    if (!Read32bitsFloat(creature.mSlapFearlessDuration))
        return false;
    SKIP_FSTREAM_U32(mFileStream);
    SKIP_FSTREAM_U32(mFileStream);
    SKIP_FSTREAM_U16(mFileStream); // possessionManaCost
    SKIP_FSTREAM_U16(mFileStream); // ownLandHealthIncrease
    if (!Read32bitsFloat(creature.mMeleeRange))
        return false;
    SKIP_FSTREAM_U32(mFileStream);
    if (!Read32bitsFloat(creature.mTortureTimeToConvert))
        return false;
    if (!Read32bitsFloat(creature.mMeleeRecharge))
        return false;
    if (!ReadCreatureFlags(creature))
        return false;
    unsigned short expForNextLevel;
    READ_FROM_FSTREAM(mFileStream, expForNextLevel);
    unsigned char jobClass;
    READ_FROM_FSTREAM(mFileStream, jobClass);

    if (!KWDParseENUM(jobClass, creature.mJobClass))
        return false;

    unsigned char fightStyle;
    READ_FROM_FSTREAM(mFileStream, fightStyle);
    unsigned short expPerSecond;
    READ_FROM_FSTREAM(mFileStream, expPerSecond);
    unsigned short expPerSecondTraining;
    READ_FROM_FSTREAM(mFileStream, expPerSecondTraining);
    unsigned short researchPerSecond;
    READ_FROM_FSTREAM(mFileStream, researchPerSecond);
    unsigned short manufacturePerSecond;
    READ_FROM_FSTREAM(mFileStream, manufacturePerSecond);
    READ_FSTREAM_U16(mFileStream, creature.mHp);
    READ_FSTREAM_U16(mFileStream, creature.mHpFromChicken);
    READ_FSTREAM_U16(mFileStream, creature.mFear);
    READ_FSTREAM_U16(mFileStream, creature.mThreat);
    READ_FSTREAM_U16(mFileStream, creature.mMeleeDamage);
    READ_FSTREAM_U16(mFileStream, creature.mSlapDamage);
    READ_FSTREAM_U16(mFileStream, creature.mManaGenPrayer);
    SKIP_FSTREAM_U16(mFileStream);
    READ_FSTREAM_U16(mFileStream, creature.mPay);
    READ_FSTREAM_U16(mFileStream, creature.mMaxGoldHeld);
    SKIP_FSTREAM_U16(mFileStream); // float16?
    READ_FSTREAM_U16(mFileStream, creature.mDecomposeValue);
    READ_FSTREAM_U16(mFileStream, creature.mNameStringId);
    READ_FSTREAM_U16(mFileStream, creature.mTooltipStringId);
    READ_FSTREAM_U16(mFileStream, creature.mAngerNoLair);
    READ_FSTREAM_U16(mFileStream, creature.mAngerNoFood);
    READ_FSTREAM_U16(mFileStream, creature.mAngerNoPay);
    READ_FSTREAM_U16(mFileStream, creature.mAngerNoWork);
    READ_FSTREAM_U16(mFileStream, creature.mAngerSlap);
    READ_FSTREAM_U16(mFileStream, creature.mAngerInHand);
    READ_FSTREAM_U16(mFileStream, creature.mInitialGoldHeld);
    SKIP_FSTREAM_U16(mFileStream); // entrance effect id
    READ_FSTREAM_U16(mFileStream, creature.mGeneralDescriptionStringId);
    READ_FSTREAM_U16(mFileStream, creature.mStrengthStringId);
    READ_FSTREAM_U16(mFileStream, creature.mWeaknessStringId);
    SKIP_FSTREAM_U16(mFileStream); // slap effect id
    SKIP_FSTREAM_U16(mFileStream); // death effect id
    SKIP_FSTREAM_U8(mFileStream); // melee1Swipe,  Swipes, 1st person attacks
    SKIP_FSTREAM_U8(mFileStream); // melee2Swipe
    SKIP_FSTREAM_U8(mFileStream); // melee3Swipe
    SKIP_FSTREAM_U8(mFileStream); // spellSwipe
    SKIP_FSTREAM_U8(mFileStream); // firstPersonSpecialAbility1
    SKIP_FSTREAM_U8(mFileStream); // firstPersonSpecialAbility2
    SKIP_FSTREAM_BYTES(mFileStream, 3);
    READ_FSTREAM_U8(mFileStream, creature.mCreatureTypeId);
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream); // hungerFill
    SKIP_FSTREAM_U8(mFileStream); // unhappy threshold
    SKIP_FSTREAM_U8(mFileStream); // meleeAttackType
    SKIP_FSTREAM_U8(mFileStream);
    READ_FSTREAM_U8(mFileStream, creature.mLairObjectId);
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream); // death fall direction
    SKIP_FSTREAM_U8(mFileStream);
    if (!ReadString8(32, creature.mSoundCategory))
        return false;

    unsigned char armorMaterial;
    READ_FSTREAM_U8(mFileStream, armorMaterial);
    if (!KWDParseENUM(armorMaterial, creature.mArmourType))
        return false;

    if (!ReadArtResource(creature.mFirstPersonFilterResource))
        return false;

    SKIP_FSTREAM_U16(mFileStream);

    float unknownFloat;
    if (!Read32bitsFloat(unknownFloat))
        return false;

    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_DrunkIdle]))
        return false;

    SKIP_FSTREAM_U8(mFileStream); // special1swipe
    SKIP_FSTREAM_U8(mFileStream); // special2swipe

    if (!ReadArtResource(creature.mFirstPersonMeleeResource))
        return false;

    SKIP_FSTREAM_U32(mFileStream);

    READ_FSTREAM_I16(mFileStream, creature.mTortureHpChange);
    READ_FSTREAM_I16(mFileStream, creature.mTortureMoodChange);

    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Melee2]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Special4]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Special5]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Special6]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Special7]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Special8]))
        return false;

    // 7 anim offsets
    cxx_assert(CountOf(creature.mAnimationOffsets) == 7);
    for (int i = 0; i < CountOf(creature.mAnimationOffsets); ++i)
    {
        if (!ReadVector3f(creature.mAnimationOffsets[i]))
            return false;
    }
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_WalkBack]))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 48 * 4);

    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Pose_Frame]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Walk2]))
        return false;
    if (!ReadArtResource(creature.mAnimationResources[CreatureAnimation_Die_Pose]))
        return false;

    READ_FSTREAM_U16(mFileStream, creature.mUniqueNameTextId);

    SKIP_FSTREAM_BYTES(mFileStream, 2 * 4);

    SKIP_FSTREAM_U32(mFileStream); // firstPersonSpecialAbility1Count
    SKIP_FSTREAM_U32(mFileStream); // firstPersonSpecialAbility2Count

    if (!ReadArtResource(creature.mUniqueResource))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadCreaturesData(const KWDFileHeader& header, ScenarioDefinition& scenarioData) 
{
    scenarioData.mCreatureDefs.resize(header.mItemsCount + 1);
    scenarioData.mCreatureDefs[0] = {}; // dummy element

    std::streamoff startoffset = mFileStream.tellg();
    std::streamoff elementSize = header.mContentSize / header.mItemsCount;
    // read definitions
    for (int icreature = 1; icreature < header.mItemsCount + 1; ++icreature)
    {
        std::streamoff itemoffset = mFileStream.tellg();
        if (!ReadCreaturesDefinition(scenarioData.mCreatureDefs[icreature]))
            return false;

        std::streamoff deltaoffset = mFileStream.tellg() - itemoffset;
        cxx_assert(deltaoffset <= elementSize);
        if (deltaoffset < elementSize)
        {
            mFileStream.seekg(elementSize - deltaoffset, std::ios::cur);
        }
        
    }
    std::streamoff endoffset = mFileStream.tellg();

    cxx_assert(endoffset - startoffset == header.mContentSize);
    return true;
}

bool DK2ScenarioReader::ReadTerrainDefinition(TerrainDefinition& terrainDef)
{
    terrainDef = {};

    if (!ReadString8(32, terrainDef.mName))
        return false;

    // complete resource
    if (!ReadArtResource(terrainDef.mResourceComplete))
        return false;

    // side resource
    if (!ReadArtResource(terrainDef.mResourceSide))
        return false;

    // top resource
    if (!ReadArtResource(terrainDef.mResourceTop))
        return false;

    // tagged top resource
    if (!ReadArtResource(terrainDef.mResourceTagged))
        return false;

    if (!ReadStringId())
        return false;

    unsigned int depthInt;
    READ_FROM_FSTREAM(mFileStream, depthInt);

    if (!Read32bitsFloat(terrainDef.mLightHeight))
        return false;

    if (!ReadTerrainFlags(terrainDef))
        return false;

    READ_FSTREAM_U16(mFileStream, terrainDef.mDamage);

    unsigned short filledWord;
    READ_FROM_FSTREAM(mFileStream, filledWord);
    READ_FROM_FSTREAM(mFileStream, filledWord);

    READ_FSTREAM_U16(mFileStream, terrainDef.mGoldCapacity);
    READ_FSTREAM_U16(mFileStream, terrainDef.mManaGain);
    READ_FSTREAM_U16(mFileStream, terrainDef.mManaGainMax);

    unsigned short toolTipStringId;
    READ_FROM_FSTREAM(mFileStream, toolTipStringId);

    unsigned short nameStringId;
    READ_FROM_FSTREAM(mFileStream, nameStringId);

    unsigned short maxHealthEffectId;
    READ_FROM_FSTREAM(mFileStream, maxHealthEffectId);

    unsigned short destroyedEffectId;
    READ_FROM_FSTREAM(mFileStream, destroyedEffectId);

    unsigned short generalDescriptionStringId;
    READ_FROM_FSTREAM(mFileStream, generalDescriptionStringId);

    unsigned short strengthStringId;
    READ_FROM_FSTREAM(mFileStream, strengthStringId);

    unsigned short weaknessStringId;
    READ_FROM_FSTREAM(mFileStream, weaknessStringId);

    SKIP_FSTREAM_BYTES(mFileStream, 16 * sizeof(unsigned short)); // unknown

    unsigned char wibbleH;
    READ_FROM_FSTREAM(mFileStream, wibbleH);

    unsigned char leanH[3];
    for (unsigned char& entryLeanH : leanH)
    {
        READ_FROM_FSTREAM(mFileStream, entryLeanH);
    }

    unsigned char wibbleV;
    READ_FROM_FSTREAM(mFileStream, wibbleV);

    unsigned char leanV[3];
    for (unsigned char& entryLeanV : leanV)
    {
        READ_FROM_FSTREAM(mFileStream, entryLeanV);
    }

    READ_FSTREAM_U8(mFileStream, terrainDef.mTerrainType);
    READ_FSTREAM_U16(mFileStream, terrainDef.mHealthInitial);
    READ_FSTREAM_U8(mFileStream, terrainDef.mBecomesTerrainTypeWhenMaxHealth);
    READ_FSTREAM_U8(mFileStream, terrainDef.mBecomesTerrainTypeWhenDestroyed);

    unsigned char colorComponents[3];
    // terrain color
    READ_FSTREAM_U8(mFileStream, colorComponents[0]);
    READ_FSTREAM_U8(mFileStream, colorComponents[1]);
    READ_FSTREAM_U8(mFileStream, colorComponents[2]);
    terrainDef.mTerrainColor.SetComponentsF(
        (terrainDef.mTerrainColorR) ? ((colorComponents[0] + 256) / 512.0f) : (colorComponents[0] / 256.0f),
        (terrainDef.mTerrainColorG) ? ((colorComponents[1] + 256) / 512.0f) : (colorComponents[1] / 256.0f),
        (terrainDef.mTerrainColorB) ? ((colorComponents[2] + 256) / 512.0f) : (colorComponents[2] / 256.0f),
        1.0f
    );

    READ_FSTREAM_U8(mFileStream, terrainDef.mTextureFrames);

    std::string soundCategory;
    if (!ReadString8(32, soundCategory))
        return false;

    READ_FSTREAM_U16(mFileStream, terrainDef.mHealthMax);

    // ambient color
    READ_FSTREAM_U8(mFileStream, colorComponents[0]);
    READ_FSTREAM_U8(mFileStream, colorComponents[1]);
    READ_FSTREAM_U8(mFileStream, colorComponents[2]);
    terrainDef.mAmbientColor.SetComponentsF(
        (terrainDef.mAmbientColorR) ? ((colorComponents[0] + 256) / 512.0f) : (colorComponents[0] / 256.0f),
        (terrainDef.mAmbientColorG) ? ((colorComponents[1] + 256) / 512.0f) : (colorComponents[1] / 256.0f),
        (terrainDef.mAmbientColorB) ? ((colorComponents[2] + 256) / 512.0f) : (colorComponents[2] / 256.0f),
        1.0f
    );

    std::string soundCategoryFirstPerson;
    if (!ReadString8(32, soundCategoryFirstPerson))
        return false;

    unsigned int fillerDword;
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    return true;
}

bool DK2ScenarioReader::ReadCreatureFlags(CreatureDefinition& creatureDef)
{
    unsigned int flags;
    READ_FROM_FSTREAM(mFileStream, flags);

    creatureDef.mIsWorker = CHECK_BIT_ON(flags, 0);
    creatureDef.mCanBePickedUp = CHECK_BIT_ON(flags, 1);
    creatureDef.mCanBeSlapped = CHECK_BIT_ON(flags, 2);
    creatureDef.mAlwaysFlee = CHECK_BIT_ON(flags, 3);
    creatureDef.mCanWalkOnWater = CHECK_BIT_ON(flags, 4);
    creatureDef.mCanWalkOnLava = CHECK_BIT_ON(flags, 5);
    creatureDef.mCanDisarmTraps = CHECK_BIT_ON(flags, 6);
    creatureDef.mIsEvil = CHECK_BIT_ON(flags, 7);
    creatureDef.mIsImmuneToTurncoat = CHECK_BIT_ON(flags, 9);
    creatureDef.mAvailableViaPortal = CHECK_BIT_ON(flags, 10);
    creatureDef.mCanFly = CHECK_BIT_ON(flags, 13);
    creatureDef.mIsHorny = CHECK_BIT_ON(flags, 14);
    creatureDef.mLeavesCorpse = CHECK_BIT_ON(flags, 15);
    creatureDef.mCanBeHypnotized = CHECK_BIT_ON(flags, 16);
    creatureDef.mIsImmuneToChicken = CHECK_BIT_ON(flags, 17);
    creatureDef.mIsFearless = CHECK_BIT_ON(flags, 18);
    creatureDef.mCanBeElectrocuted = CHECK_BIT_ON(flags, 19);
    creatureDef.mNeedBodyForFightIdle = CHECK_BIT_ON(flags, 20);
    creatureDef.mNotTrainWhenIdle = CHECK_BIT_ON(flags, 21);
    creatureDef.mOnlyAttackableByHorny = CHECK_BIT_ON(flags, 22);
    creatureDef.mCanBeResurrected = CHECK_BIT_ON(flags, 23);
    creatureDef.mDoesntGetAngryWithEnemies = CHECK_BIT_ON(flags, 24);
    creatureDef.mFreesFriendsOnJailbreak = CHECK_BIT_ON(flags, 25);
    creatureDef.mRevealsAdjacentTraps = CHECK_BIT_ON(flags, 26);
    creatureDef.mIsUnique = CHECK_BIT_ON(flags, 27);
    creatureDef.mIsMale = CHECK_BIT_ON(flags, 31);

    return true;
}

bool DK2ScenarioReader::ReadCreatureJobAlternative()
{
    unsigned int jobType;
    READ_FROM_FSTREAM(mFileStream, jobType);

    eCreatureJob creatureJob;
    if (!KWDParseENUM(jobType, creatureJob))
    {
        cxx_assert(false);
    }

    unsigned short moodChange;
    READ_FROM_FSTREAM(mFileStream, moodChange);
    unsigned short manaChange;
    READ_FROM_FSTREAM(mFileStream, manaChange);
    return true;
}

bool DK2ScenarioReader::ReadCreatureJobPreference()
{
    unsigned int jobType;
    READ_FROM_FSTREAM(mFileStream, jobType);

    eCreatureJob creatureJob;
    if (!KWDParseENUM(jobType, creatureJob))
    {
        cxx_assert(false);
    }

    unsigned short moodChange;
    READ_FROM_FSTREAM(mFileStream, moodChange);
    unsigned short manaChange;
    READ_FROM_FSTREAM(mFileStream, manaChange);
    unsigned char chance;
    READ_FROM_FSTREAM(mFileStream, chance);
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream);
    return true;
}

bool DK2ScenarioReader::ReadCreatureResistance()
{
    unsigned char attackType;
    READ_FROM_FSTREAM(mFileStream, attackType);
    unsigned char value;
    READ_FROM_FSTREAM(mFileStream, value);
    return true;
}

bool DK2ScenarioReader::ReadCreatureSpell()
{
    glm::vec3 shotOffset;
    if (!ReadVector3f(shotOffset))
        return false;

    SKIP_FSTREAM_U8(mFileStream);
    unsigned char playAnimation;
    READ_FROM_FSTREAM(mFileStream, playAnimation);
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream);
    float shotDelay;
    if (!Read32bitsFloat(shotDelay))
        return false;
    SKIP_FSTREAM_U8(mFileStream);
    SKIP_FSTREAM_U8(mFileStream);
    unsigned char creatureSpellId;
    READ_FROM_FSTREAM(mFileStream, creatureSpellId);
    unsigned char levelAvailable;
    READ_FROM_FSTREAM(mFileStream, levelAvailable);
    return true;
}

bool DK2ScenarioReader::ReadCreatureAttraction()
{
    unsigned int present;
    READ_FROM_FSTREAM(mFileStream, present);
    unsigned short roomId;
    READ_FROM_FSTREAM(mFileStream, roomId);
    unsigned short roomSize;
    READ_FROM_FSTREAM(mFileStream, roomSize);
    return true;
}

bool DK2ScenarioReader::ReadTerrainData(int numElements, ScenarioDefinition& scenarioData)
{
    scenarioData.mTerrainDefs.resize(numElements + 1);
    scenarioData.mTerrainDefs[0] = {}; // dummy element

    // read definitions
    for (int ielement = 1; ielement < numElements + 1; ++ielement)
    {
        if (!ReadTerrainDefinition(scenarioData.mTerrainDefs[ielement]))
            return false;

        bool correctId = (scenarioData.mTerrainDefs[ielement].mTerrainType == ielement);
        cxx_assert(correctId);
    }
    return true;
}

bool DK2ScenarioReader::ReadLevelVariables(ScenarioDefinition& scenarioData)
{
    unsigned short fillerWord;
    unsigned int fillerDword;

    READ_FROM_FSTREAM(mFileStream, fillerWord); // trigger id

    unsigned short ticksPerSecond;
    READ_FROM_FSTREAM(mFileStream, ticksPerSecond);

    scenarioData.mTicksPerSecond = 4.0f; // fixed tick rate

    SKIP_FSTREAM_BYTES(mFileStream, 520); // unknown data

    // read text messages
    std::vector<std::wstring> levelMessages;
    levelMessages.resize(20);
    for (std::wstring& messageEntry: levelMessages)
    {
        if (!ReadString(512, messageEntry))
            return false;
    }

    READ_FROM_FSTREAM(mFileStream, fillerWord); // flags

    std::string speechString;
    if (!ReadString8(32, speechString))
        return false;

    unsigned char talismanPieces;
    READ_FROM_FSTREAM(mFileStream, talismanPieces);
    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    unsigned char soundtrack;
    unsigned char textTableId;
    READ_FROM_FSTREAM(mFileStream, soundtrack);
    READ_FROM_FSTREAM(mFileStream, textTableId);
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textTitleId
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textPlotId
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textDebriefId
    READ_FROM_FSTREAM(mFileStream, fillerWord); // textObjectvId
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord); // speclvlIdx

    // unknown data
    SKIP_FSTREAM_BYTES(mFileStream, 8 * sizeof(unsigned char));
    SKIP_FSTREAM_BYTES(mFileStream, 8 * sizeof(unsigned short));

    // path
    std::string terrainPath;
    if (!ReadString8(32, terrainPath))
        return false;

    unsigned char oneShotHornyLev;
    unsigned char fillerByte;
    READ_FROM_FSTREAM(mFileStream, oneShotHornyLev);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);
    READ_FROM_FSTREAM(mFileStream, fillerByte);

    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);
    READ_FROM_FSTREAM(mFileStream, fillerWord);

    std::wstring heroName;
    if (!ReadString(32, heroName))
        return false;

    return true;
}

bool DK2ScenarioReader::ReadMapInfo(ScenarioDefinition& scenarioData, std::vector<LevelDataFilePath>& paths)
{
    mFileStream.seekg(20); // end of header

    // additional header data

    unsigned short pathCount;
    unsigned short unknownCount;
    unsigned int fillerDword;

    READ_FROM_FSTREAM(mFileStream, pathCount);
    READ_FROM_FSTREAM(mFileStream, unknownCount);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    // timestamp 1
    if (!ReadTimestamp())
        return false;

    // timestamp 2
    if (!ReadTimestamp())
        return false;

    READ_FROM_FSTREAM(mFileStream, fillerDword);
    READ_FROM_FSTREAM(mFileStream, fillerDword);

    // property data
    if (!ReadString(64, scenarioData.mLevelName))
        return false;

    if (!ReadString(1024, scenarioData.mLevelDescription))
        return false;

    if (!ReadString(64, scenarioData.mLevelAuthor))
        return false;

    if (!ReadString(64, scenarioData.mLevelEmail))
        return false;

    if (!ReadString(1024, scenarioData.mLevelInformation))
        return false;

    // variables
    if (!ReadLevelVariables(scenarioData))
        return false;

    SKIP_FSTREAM_BYTES(mFileStream, 8); // unknown data

    // read paths
    paths.resize(pathCount);
    for (LevelDataFilePath& pathEntry : paths)
    {
        READ_FSTREAM_U32(mFileStream, pathEntry.nDataTypeId);
        READ_FROM_FSTREAM(mFileStream, fillerDword);

        if (!ReadString8(64, pathEntry.mFilePath))
            return false;
    }
    // unknown data
    SKIP_FSTREAM_BYTES(mFileStream, unknownCount * sizeof(unsigned short));
    return !!mFileStream;
}

bool DK2ScenarioReader::ReadDataFile(KWDFileHeader& fileHeader, ScenarioDefinition& scenarioData)
{
    // read header
    READ_FSTREAM_U32(mFileStream, fileHeader.mTypeId);

    unsigned int byteSize = 0;
    unsigned int checkOne = 0;
    READ_FSTREAM_U32(mFileStream, byteSize);
    READ_FSTREAM_U32(mFileStream, fileHeader.mFileSize);
    READ_FSTREAM_U32(mFileStream, checkOne);
    READ_FSTREAM_U32(mFileStream, fileHeader.mHeaderEndOffset);

    cxx_assert(byteSize == sizeof(unsigned int));

    switch (fileHeader.mTypeId)
    {
        case DKLD_MAP:
        {
            READ_FROM_FSTREAM(mFileStream, scenarioData.mLevelDimensionX);
            READ_FROM_FSTREAM(mFileStream, scenarioData.mLevelDimensionY);           
        }
        break;
        case DKLD_TRIGGERS:
        {
            SKIP_FSTREAM_BYTES(mFileStream, 4); //itemcount 1
            SKIP_FSTREAM_BYTES(mFileStream, 4); //itemcount 2
            SKIP_FSTREAM_BYTES(mFileStream, 4); //unknown

            if (!ReadTimestamp()) // created
                return false;
            if (!ReadTimestamp()) // modified
                return false;
        }
        break;
        case DKLD_LEVEL:
        {
            READ_FSTREAM_U16(mFileStream, fileHeader.mItemsCount);
            SKIP_FSTREAM_BYTES(mFileStream, 2); // height
            SKIP_FSTREAM_BYTES(mFileStream, 4); // unknown

            if (!ReadTimestamp()) // created
                return false;
            if (!ReadTimestamp()) // modified
                return false;
        }
        break;
        default:
        {
            READ_FSTREAM_U32(mFileStream, fileHeader.mItemsCount);
            SKIP_FSTREAM_BYTES(mFileStream, 4); // unknown

            if (!ReadTimestamp()) // created
                return false;
            if (!ReadTimestamp()) // modified
                return false;
        }
        break;
    }

    unsigned int checkTwo;
    READ_FSTREAM_U32(mFileStream, checkTwo);
    READ_FSTREAM_U32(mFileStream, fileHeader.mContentSize);

    // read body
    switch (fileHeader.mTypeId)
    {
        case DKLD_GLOBALS:
        break;
        case DKLD_MAP:
        {
            if (!ReadMapData(scenarioData))
                return false;
        }
        break;
        case DKLD_TERRAIN:
        {
            if (!ReadTerrainData(fileHeader.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_ROOMS:
        {
            if (!ReadRoomsData(fileHeader.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_TRAPS:
        break;
        case DKLD_DOORS:
        break;
        case DKLD_KEEPER_SPELLS:
        break;
        case DKLD_CREATURE_SPELLS:
        break;
        case DKLD_CREATURES:
        {
            if (!ReadCreaturesData(fileHeader, scenarioData))
                return false;
        }
        break;
        case DKLD_PLAYERS:
        {
            if (!ReadPlayersData(fileHeader.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_THINGS:
        {
            if (!ReadThingsData(fileHeader.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_TRIGGERS:
        break;
        case DKLD_LEVEL:
        break;
        case DKLD_VARIABLES:
        {
            if (!ReadScenarioVariables(fileHeader.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_OBJECTS:
        {
            if (!ReadObjectsData(fileHeader.mItemsCount, scenarioData))
                return false;
        }
        break;
        case DKLD_EFFECT_ELEMENTS:
        break;
        case DKLD_SHOTS:
        break;
        case DKLD_EFFECTS:
        break;
    }

    return true;
}

bool DK2ScenarioReader::ReadDataFile(ScenarioDefinition& scenarioData)
{
    for (;;)
    {
        KWDFileHeader fileHeader;

        std::streamoff startoffset = mFileStream.tellg();
        if (!ReadDataFile(fileHeader, scenarioData))
            return false;

        if (!mFileStream.seekg(startoffset + fileHeader.mFileSize, std::ios::beg))
            break;

        if ((mFileStream.peek() == std::ifstream::traits_type::eof()) ||
            mFileStream.eof())
        {
            break;
        }
    }
    return true;
}

bool DK2ScenarioReader::ExploreTerrainTypes(ScenarioDefinition& scenarioData) const
{
    // all definitions are loaded at this point, so we should map rooms to terrain types
    bool roomAndTerrainDefinitionsNotNull = !(scenarioData.mRoomDefs.empty() || scenarioData.mTerrainDefs.empty());
    if (!roomAndTerrainDefinitionsNotNull)
        return false;

    scenarioData.mRoomByTerrainType.resize(scenarioData.mTerrainDefs.size(), RoomTypeId_Null);
        
    // explore each room definition
    for (RoomDefinition& roomDefinition : scenarioData.mRoomDefs)
    {
        // null definition is being skipped
        if (roomDefinition.mRoomType == RoomTypeId_Null)
            continue;

        // bind identifier
        scenarioData.mRoomByTerrainType[roomDefinition.mTerrainType] = roomDefinition.mRoomType;
    }

    // find special terrain types
    for (TerrainDefinition& terrainDefinition : scenarioData.mTerrainDefs)
    {
        // null definition is being skipped
        if (terrainDefinition.mTerrainType == TerrainTypeId_Null)
            continue;

        if (terrainDefinition.mIsLava)
        {
            scenarioData.mLavaTerrainType = terrainDefinition.mTerrainType;
        }

        if (terrainDefinition.mIsWater)
        {
            scenarioData.mWaterTerrainType = terrainDefinition.mTerrainType;
        }

        if (terrainDefinition.mPlayerColouredPath && (scenarioData.mPlayerColouredPathTerrainType == TerrainTypeId_Null))
        {
            if (scenarioData.mRoomByTerrainType[terrainDefinition.mTerrainType] == RoomTypeId_Null)
            {
                scenarioData.mPlayerColouredPathTerrainType = terrainDefinition.mTerrainType;
            }
        }

        if (terrainDefinition.mPlayerColouredWall)
        {
            scenarioData.mPlayerColouredWallTerrainType = terrainDefinition.mTerrainType;
        }
    }

    return true;
}

void DK2ScenarioReader::ApplyExtensions(JsonDocument& extensionsData, ScenarioDefinition& scenarioData) const
{
    // 1 fix terrain resources

    // hero lair complete resource is missed, seems it originally hardcoded in game exe
    const int HeroLairTerrainIdentifier = 35;

    cxx_assert(HeroLairTerrainIdentifier < scenarioData.mTerrainDefs.size());
    TerrainDefinition& terrainDef = scenarioData.mTerrainDefs[HeroLairTerrainIdentifier];
    if (!terrainDef.mResourceComplete.IsDefined())
    {
        terrainDef.mResourceComplete.mResourceType = eArtResource_TerrainMesh;
        terrainDef.mResourceComplete.mResourceName = "hero_outpost_floor";
    }
    else
    {
        cxx_assert(false);
    }
    // does not use player colors
    terrainDef.mPlayerColouredPath = false;

    // 2 define room pillars

    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_WorkShop))
    {
        definition->mPillarObjectId = GameObjectClassId_WorkshopPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Treasury))
    {
        definition->mPillarObjectId = GameObjectClassId_TreasuryPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Hatchery))
    {
        definition->mPillarObjectId = GameObjectClassId_HatcheryPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Casino))
    {
        definition->mPillarObjectId = GameObjectClassId_CasinoPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_CombatPit))
    {
        definition->mPillarObjectId = GameObjectClassId_PitPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Graveyard))
    {
        definition->mPillarObjectId = GameObjectClassId_GraveyardPillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_TortureChamber))
    {
        definition->mPillarObjectId = GameObjectClassId_TorturePillar;
    }
    if (RoomDefinition* definition = scenarioData.GetRoomDefinition(RoomTypeId_Temple))
    {
        definition->mPillarObjectId = GameObjectClassId_TempleCandlestick;
    }

    // 3 enable walls for hero gate rooms
    for (RoomDefinition& roller: scenarioData.mRoomDefs)
    {
        if (roller.mHasWalls) continue;
        if ((roller.mTileConstruction == eRoomTileConstruction_HeroGateFrontend) ||
            (roller.mTileConstruction == eRoomTileConstruction_HeroGate_3_by_1))
        {
            roller.mHasWalls = true; 
        }
    }

    // 3 creatures
    if (JsonElement creaturesRoot = extensionsData.GetRootElement().FindElement("creatures"))
    {
        cxx_assert(creaturesRoot.IsObject());

        for (CreatureDefinition& roller: scenarioData.mCreatureDefs)
        {
            if (roller.mCreatureName.empty()) continue;
            if (JsonElement creatureExtData = creaturesRoot.FindElement(roller.mCreatureName))
            {
                // animations
                float walkAnimScale = 1.0f;
                if (JsonQuery(creatureExtData, "walk_anim_scale", walkAnimScale) && (walkAnimScale > 0.0f))
                {
                    roller.mWalkAnimScale = walkAnimScale;
                }
            }
        }
    }
}

bool DK2ScenarioReader::ReadScenarioData(const std::string& filePath, ScenarioDefinition& scenarioData)
{
    const std::string levelName = FSGetFileNameWithoutExtension(filePath);

    // open file stream
    mFileStream.open(filePath, std::ios::in | std::ios::binary);
    if (!mFileStream.is_open())
        return false;

    std::vector<LevelDataFilePath> paths;
    if (!ReadMapInfo(scenarioData, paths))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Error reading scenario data info from '%s'", levelName.c_str());
        return false;
    }

    mFileStream.close();

    // read data from data files
    for (const LevelDataFilePath& pathEntry: paths)
    {
        std::string dataFilePath;

        bool isPathLocated = gFiles.LocateMapData(pathEntry.mFilePath, dataFilePath);
        if (isPathLocated && (pathEntry.nDataTypeId == DKLD_GLOBALS))
        {
            gConsole.LogMessage(eLogLevel_Info, "Scanario overrides globals");
        }

        if (!isPathLocated)
        {
            if (pathEntry.nDataTypeId != DKLD_GLOBALS)
            {
                gConsole.LogMessage(eLogLevel_Warning, "Cannot locate scenario data file '%s'", pathEntry.mFilePath.c_str());
            }
            continue;
        }

        mFileStream.open(dataFilePath, std::ios::in | std::ios::binary);
        if (mFileStream.is_open())
        {
            if (!ReadDataFile(scenarioData))
            {
                gConsole.LogMessage(eLogLevel_Warning, "Error reading scenario data file '%s'", pathEntry.mFilePath.c_str());
                return false;
            }
            mFileStream.close();
        }
    }

    if (!ExploreTerrainTypes(scenarioData))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Error exploring scenario terrain types");
        return false;
    }

    std::string extensionsFilePath;
    JsonDocument extensionsData;
    if (!gFiles.PathToFile("gameplay/extensions.json", extensionsFilePath) || !FSLoadJSON(extensionsFilePath, extensionsData))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Error loading gameplay extensions data");
    }

    ApplyExtensions(extensionsData, scenarioData);
    return true;
}
