#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "CreatureDefs.h"
#include "GameObjectDefs.h"

//////////////////////////////////////////////////////////////////////////

// Defines some timestamp
struct ScenarioTimestamp
{
public:
    int mYear {};
    int mDay {};
    int mMonth {};
    int mHour {};
    int mMinute {};
    int mSeconds {};
};

//////////////////////////////////////////////////////////////////////////

enum eThingMaterialType
{
    eThingMaterialType_None,
    eThingMaterialType_Flesh,
    eThingMaterialType_Rock,
    eThingMaterialType_Wood,
    eThingMaterialType_Metal1,
    eThingMaterialType_Metal2,
    eThingMaterialType_Magic,
    eThingMaterialType_Glass,
    eThingMaterialType_COUNT
};

//////////////////////////////////////////////////////////////////////////
// Scenario Art Resource Definition
//////////////////////////////////////////////////////////////////////////

enum eArtResource 
{
    eArtResource_Null,
    eArtResource_Sprite,
    eArtResource_Alpha,
    eArtResource_AdditiveAlpha,
    eArtResource_TerrainMesh,
    eArtResource_Mesh,
    eArtResource_AnimatingMesh,
    eArtResource_ProceduralMesh,
    eArtResource_MeshCollection,
    eArtResource_COUNT
};

enum_serialize_decl(eArtResource);

struct ArtResourceDefinition 
{
public:
    ArtResourceDefinition()
        : mAnimationDesc{}
    {}

    // Test whether resource is defined
    inline bool IsDefined() const { return mResourceType > eArtResource_Null; }
    inline bool HasResource(eArtResource resourceType) const
    {
        return mResourceType == resourceType;
    }

    // ArtResource detailed description depends on resource type
    union
    {
        struct {
            int mFrames;
            float mWidth;
            float mHeight;
        } mImageDesc;
        struct {
            int mFrames;
            float mScale;
        } mMeshDesc;
        struct {
            int mFrames;
            int mFps;
            int mDistStart;
            int mDistEnd;
        } mAnimationDesc;
        struct {
            int mId;
        } mProcDesc;
        struct {
            int mFrames;
        } mTerrainDesc;
    };

    // art resource information
    eArtResource mResourceType = eArtResource_Null;
    std::string mResourceName;
    int mStartAF {};
    int mEndAF {};
    bool mPlayerColoured {};
    bool mAnimatingTexture {};
    bool mHasStartAnimation {};
    bool mHasEndAnimation {};
    bool mRandomStartFrame {};
    bool mOriginAtBottom {};
    bool mDoesntLoop {};
    bool mFlat {};
    bool mDoesntUseProgressiveMesh {};
    bool mUseAnimatingTextureForSelection {};
    bool mPreload {};
    bool mBlood {};
};
    
//////////////////////////////////////////////////////////////////////////
// Player definition
//////////////////////////////////////////////////////////////////////////

struct PlayerDefinition
{
public:
    PlayerDefinition() = default;

public:
    ePlayerID mPlayerId = ePlayerID_Null;
    ePlayerType mPlayerType;
    std::string mPlayerName;
    int mStartCameraX {};
    int mStartCameraY {};
    int mInitialGold {};
    int mInitialMana {};
    int mManaValue {};
    int mGoldValue {};
    eComputerAI mComputerAI = eComputerAI_MasterKeeper;
    ComputerAIPreferences mAIPreferences;
};

//////////////////////////////////////////////////////////////////////////
// Map Tile Definition
//////////////////////////////////////////////////////////////////////////

// The terrain under the bridge
enum eBridgeTerrain 
{
    eBridgeTerrain_Null,
    eBridgeTerrain_Water,
    eBridgeTerrain_Lava,
    eBridgeTerrain_COUNT
};

//////////////////////////////////////////////////////////////////////////

using TerrainTypeId = unsigned int;
// well known terrain type
enum : TerrainTypeId
{ 
    TerrainTypeId_Null = 0,
    TerrainTypeId_ImpenetrableRock = 1,
    TerrainTypeId_Rock = 2,
    TerrainTypeId_DirtPath = 3,
    TerrainTypeId_Water = 4,
    TerrainTypeId_Lava = 5,
    TerrainTypeId_Gold = 6,
    TerrainTypeId_Gems = 7,
    TerrainTypeId_ClaimedPath = 8,
    TerrainTypeId_ReinforcedWall = 9,
    TerrainTypeId_Treasury = 10,
    TerrainTypeId_Lair = 11,
    TerrainTypeId_Portal = 12,
    TerrainTypeId_Hatchery = 13,
    TerrainTypeId_DungeonHeart = 14,
    TerrainTypeId_Library = 15,
    TerrainTypeId_TrainingRoom = 16,
    TerrainTypeId_WoodenBridge = 17,
    TerrainTypeId_GuardRoom = 18,
    TerrainTypeId_WorkShop = 19,
    TerrainTypeId_Prison = 20,
    TerrainTypeId_TortureChamber = 21,
    TerrainTypeId_Temple = 22,
    TerrainTypeId_Gaveyard = 23,
    TerrainTypeId_Casino = 24,
    TerrainTypeId_CombatPit = 25,
    TerrainTypeId_StoneBridge = 26,
    TerrainTypeId_FogOfWar = 27,
    TerrainTypeId_HeroGate_Final = 28,
    TerrainTypeId_HeroGate_Tile = 29,
    TerrainTypeId_EdgeOfMap = 30,
    TerrainTypeId_ManaVault = 31,
    TerrainTypeId_ClaimedVault = 32,
    TerrainTypeId_HeroGate_2x2 = 33,
    TerrainTypeId_HeroGate_Frontend = 34,
    TerrainTypeId_HeroLair = 35,
    TerrainTypeId_HeroStoneBridge = 36,
    TerrainTypeId_HeroGate_3x1 = 37,
    TerrainTypeId_ClaimScan = 38,
    TerrainTypeId_MercenaryGate = 39,
    TerrainTypeId_HeroPortal = 40,
    TerrainTypeId_Crypt = 41
};

//////////////////////////////////////////////////////////////////////////

// single map tile information
struct ScenarioMapTileDefinition
{
public:
    TerrainTypeId mTerrainType;
    ePlayerID mOwnerId; // owner
    eBridgeTerrain mTerrainUnderTheBridge;
};

//////////////////////////////////////////////////////////////////////////
// Terrain Type Definition
//////////////////////////////////////////////////////////////////////////

struct TerrainDefinition 
{
public:
    TerrainDefinition()
        : mTerrainType(TerrainTypeId_Null)
        , mBecomesTerrainTypeWhenMaxHealth(TerrainTypeId_Null)
        , mBecomesTerrainTypeWhenDestroyed(TerrainTypeId_Null)
        , mIsSolid()
        , mIsImpenetrable()
        , mIsOwnable()
        , mIsTaggable()
        , mIsAttackable()
        , mHasTorch()
        , mIsWater()
        , mIsLava()
        , mAlwaysExplored()
        , mPlayerColouredPath()
        , mPlayerColouredWall()  
        , mConstructionTypeWater()
        , mConstructionTypeQuad()
        , mUnexploreIfDugByAnotherPlayer()
        , mFillInable()
        , mAllowRoomWalls()
        , mIsDecay()
        , mHasRandomTexture()
        , mTerrainColorR()
        , mTerrainColorG()
        , mTerrainColorB()
        , mDwarfCanDigThrough()
        , mRevealThroughFogOfWar()
        , mAmbientColorR()
        , mAmbientColorG()
        , mAmbientColorB()
        , mHasLight()
        , mHasAmbientLight()
        , mTextureFrames()
        , mDamage()
        , mGoldValue()
        , mManaGain()
        , mManaGainMax()
        , mHealthInitial()
        , mHealthMax()
        , mAmbientColor(COLOR_WHITE)
        , mTerrainColor(COLOR_WHITE)
        , mLightHeight()
    {
    }

    // get cell resource shortcut
    inline ArtResourceDefinition* GetCellResource() 
    {
        ArtResourceDefinition& cellResource = (mIsSolid && !mConstructionTypeQuad) ? mResourceTop : mResourceComplete;
        cxx_assert(cellResource.IsDefined());
        return &cellResource;
    }

public:
    TerrainTypeId mTerrainType;
    std::string mName;
    TerrainTypeId mBecomesTerrainTypeWhenMaxHealth;
    TerrainTypeId mBecomesTerrainTypeWhenDestroyed;
    bool mIsSolid;
    bool mIsImpenetrable;
    bool mIsOwnable;
    bool mIsTaggable;
    bool mIsAttackable;
    bool mHasTorch;
    bool mIsWater;
    bool mIsLava;
    bool mAlwaysExplored;
    bool mPlayerColouredPath;
    bool mPlayerColouredWall;  
    bool mConstructionTypeWater;
    bool mConstructionTypeQuad;
    bool mUnexploreIfDugByAnotherPlayer;
    bool mFillInable;
    bool mAllowRoomWalls;
    bool mIsDecay;
    bool mHasRandomTexture;
    bool mTerrainColorR; 
    bool mTerrainColorG; 
    bool mTerrainColorB;
    bool mDwarfCanDigThrough;
    bool mRevealThroughFogOfWar;
    bool mAmbientColorR;
    bool mAmbientColorG; 
    bool mAmbientColorB; 
    bool mHasLight;
    bool mHasAmbientLight;
    ArtResourceDefinition mResourceComplete;
    ArtResourceDefinition mResourceSide;
    ArtResourceDefinition mResourceTop;
    ArtResourceDefinition mResourceTagged;
    int mTextureFrames;
    int mDamage;
    int mGoldValue;
    int mManaGain;
    int mManaGainMax;
    int mHealthInitial;
    int mHealthMax;
    float mLightHeight;
    Color32 mTerrainColor;
    Color32 mAmbientColor;
};

//////////////////////////////////////////////////////////////////////////
// Room Type Definition
//////////////////////////////////////////////////////////////////////////

struct RoomDefinition
{
public:
    RoomDefinition()
        : mRoomType(RoomTypeId_Null)
        , mPlaceableOnWater()
        , mPlaceableOnLava()
        , mPlaceableOnLand()
        , mHasWalls()
        , mCentre()
        , mSpecialTiles()
        , mNormalTiles()
        , mBuildable()
        , mSpecialWalls()
        , mIsAttackable()
        , mHasFlame()
        , mIsGood()
        , mRecommendedSizeX()
        , mRecommendedSizeY()
        , mTileConstruction(eRoomTileConstruction_Normal)
        , mTerrainType(TerrainTypeId_Null)
        , mCost()
        , mOrderInEditor()
        , mObjectIds()
    {
    }
public:
    RoomTypeId mRoomType;
    std::string mRoomName;
    ArtResourceDefinition mGuiIcon;
    ArtResourceDefinition mEditorIcon;
    ArtResourceDefinition mCompleteResource;
    ArtResourceDefinition mStraightResource;
    ArtResourceDefinition mInsideCornerResource;
    ArtResourceDefinition mUnknownResource;
    ArtResourceDefinition mOutsideCornerResource;
    ArtResourceDefinition mWallResource;
    ArtResourceDefinition mCapResource;
    ArtResourceDefinition mCeilingResource;
    ArtResourceDefinition mTorchResource;
    GameObjectClassId mPillarObjectId = GameObjectClassId_Null; // extension
    bool mPlaceableOnWater;
    bool mPlaceableOnLava;
    bool mPlaceableOnLand;
    bool mHasWalls;
    bool mCentre;
    bool mSpecialTiles;
    bool mNormalTiles;
    bool mBuildable;
    bool mSpecialWalls;
    bool mIsAttackable;
    bool mHasFlame;
    bool mIsGood;
    int mCost;
    int mOrderInEditor;
    GameObjectClassId mObjectIds[8];
    TerrainTypeId mTerrainType;
    TerrainDefinition* mTerrainDefinition = nullptr;
    eRoomTileConstruction mTileConstruction;
    std::string mSoundCategory;
    int mRecommendedSizeX;
    int mRecommendedSizeY;
};

//////////////////////////////////////////////////////////////////////////
// Object Type Definition
//////////////////////////////////////////////////////////////////////////

struct GameObjectDefinition
{
public:
    GameObjectDefinition() 
        : mObjectClass(GameObjectClassId_Null)
        , mDieOverTime()
        , mDieOverTimeIfNotInRoom()
        , mCanBePickedUp()
        , mCanBeSlapped()            
        , mDieWhenSlapped()
        , mCanBeDroppedOnAnyLand()
        , mObstacle()
        , mBounce()
        , mBoulderCanRollThrough()
        , mBoulderDestroys()
        , mIsPillar()
        , mDoorKey()
        , mIsDamageable()
        , mHighlightable()
        , mPlaceable()
        , mFirstPersonObstacle()
        , mSolidObstacle()
        , mCastShadows()
        , mManaValue()
        , mObjectMaterial(eThingMaterialType_None)
        , mTooltipStringId()
        , mNameStringId()
        , mSlapEffectId()
        , mDeathEffectId()
        , mMiscEffectId()
        , mStartState(eGameObjectState_None)
        , mObjectCategory(eGameObjectCategory_Normal)
    {
    }

public:
    GameObjectClassId mObjectClass;
    std::string mObjectName; 
    ArtResourceDefinition mResourceMesh; 
    ArtResourceDefinition mResourceGuiIcon;
    ArtResourceDefinition mResourceInHandIcon; 
    ArtResourceDefinition mResourceInHandMesh;
    ArtResourceDefinition mResourceUnknown;
    ArtResourceDefinition mResourceAdditional1;
    ArtResourceDefinition mResourceAdditional2;
    ArtResourceDefinition mResourceAdditional3;
    ArtResourceDefinition mResourceAdditional4;
    float mWidth;
    float mHeight;
    float mMass;
    float mSpeed;
    float mAirFriction;
    eGameObjectState mStartState;
    int mHitpoints;
    int mMaxAngle;
    int mManaValue;
    int mRoomCapacity;
    int mTooltipStringId;
    int mNameStringId;
    int mSlapEffectId;
    int mDeathEffectId;
    int mMiscEffectId;
    eThingMaterialType mObjectMaterial;
    bool mDieOverTime;
    bool mDieOverTimeIfNotInRoom;
    eGameObjectCategory mObjectCategory;
    bool mCanBePickedUp;
    bool mCanBeSlapped;             
    bool mDieWhenSlapped;
    bool mCanBeDroppedOnAnyLand;
    bool mObstacle;
    bool mBounce;
    bool mBoulderCanRollThrough;
    bool mBoulderDestroys;
    bool mIsPillar;
    bool mDoorKey;
    bool mIsDamageable;
    bool mHighlightable;
    bool mPlaceable;
    bool mFirstPersonObstacle;
    bool mSolidObstacle;
    bool mCastShadows;
    std::string mSoundCategory;
};

//////////////////////////////////////////////////////////////////////////
// Scenario variables
//////////////////////////////////////////////////////////////////////////

struct ScenarioVariables
{
public:
    int mDigRockHealth = 0;
    int mAttackTileHealth = 0;
    int mRepairTileHealth = 0;
    int mClaimTileHealth = 0;
    int mMineGoldHealth = 0;
    int mDigOwnWallHealth = 0;
    int mDigEnemyWallHealth = 0;
    int mReinforceWallHealth = 0;
    int mRepairWallHealth = 0;
    int mConvertRoomHealth = 0;
    int mAttackRoomHealth = 0;
    int mRepairRoomHealth = 0;
    int mGoldMinedFromGems = 0;
    int mMaxGoldPerTreasuryTile = 0;
    int mSpecialIncreaseGoldAmount = 0;
    int mMaxGoldPileOutsideTreasury = 0;
    int mMaxGoldPerDungeonHeartTile = 0;
    int mMaximumManaThreshold = 0;
    int mRoomSellValuePercentageOfCost = 0;
};

//////////////////////////////////////////////////////////////////////////

// Defines creature properties
class CreatureDefinition
{
public:
    CreatureDefinition()
        : mCreatureTypeId(CreatureTypeId_Null)
        , mCloneCreatureTypeId(CreatureTypeId_Null)
    {
    }

    // get animation resource with fallback to pose frame if not specified
    const ArtResourceDefinition& GetAnimResourceOrPoseFrame(CreatureAnimationID animID)
    {
        cxx_assert(animID < CreatureAnimation_COUNT);
        // fallback to pose frame
        return mAnimationResources[animID].IsDefined() ? 
            mAnimationResources[animID] : 
            mAnimationResources[CreatureAnimation_Pose_Frame];
    };

public:
    CreatureTypeId mCreatureTypeId;
    std::string mCreatureName;
	ArtResourceDefinition mAnimationResources[CreatureAnimation_COUNT];
    ArtResourceDefinition mIcon1Resource;
    ArtResourceDefinition mIcon2Resource;
    ArtResourceDefinition mPortraitResource;
    ArtResourceDefinition mFirstPersonFilterResource;
    ArtResourceDefinition mFirstPersonMeleeResource;
    ArtResourceDefinition mUniqueResource;
    int mOrderInEditor;
    int mAngerStringIdGeneral;
    float mShotDelay;
    int mOlhiEffectId;
    int mIntroductionStringId;
    float mPerceptionRange;
    int mAngerStringIdLair;
    int mAngerStringIdFood;
    int mAngerStringIdPay;
    int mAngerStringIdWork;
    int mAngerStringIdSlap;
    int mAngerStringIdHeld;
    int mAngerStringIdLonely;
    int mAngerStringIdHatred;
    int mAngerStringIdTorture;
    std::string mTranslationSoundGategory;
    std::string mSoundCategory;
    float mShuffleSpeed;
    float mHeight;
    float mEyeHeight;
    float mSpeed;
    float mRunSpeed;
    float mHungerRate;
    int mTimeSleep;
    int mTimeAwake;
    int mHp;
    int mHpFromChicken;
    int mFear;
    int mThreat;
    int mMeleeDamage;
    int mSlapDamage;
    int mManaGenPrayer;
    int mPay;
    int mMaxGoldHeld;
    int mDecomposeValue;
    int mNameStringId;
    int mTooltipStringId;
    int mAngerNoLair;
    int mAngerNoFood;
    int mAngerNoPay;
    int mAngerNoWork;
    int mAngerSlap;
    int mAngerInHand;
    int mInitialGoldHeld;
    int mGeneralDescriptionStringId;
    int mStrengthStringId;
    int mWeaknessStringId;
    float mDistanceCanSee; // tiles
    float mDistanceCanHear; // tiles
    float mStunDuration; // seconds
    float mGuardDuration; // seconds
    float mIdleDuration; // seconds
    float mSlapFearlessDuration; // seconds
    float mMeleeRange;
    float mMeleeRecharge;
    float mTortureTimeToConvert;
    // ext data begin
    float mWalkAnimScale = 1.0f;
    // ext data end
    int mTortureHpChange;
    int mTortureMoodChange;
    int mUniqueNameTextId;
    CreatureTypeId mCloneCreatureTypeId;
    glm::vec3 mAnimationOffset;
    glm::vec3 mAnimationOffsets[7];
    eCreatureJobClass mJobClass;
    eThingMaterialType mArmourType;
    GameObjectClassId mLairObjectId;
    bool mIsWorker;
    bool mCanBePickedUp;
    bool mCanBeSlapped;
    bool mAlwaysFlee;
    bool mCanWalkOnLava;
    bool mCanWalkOnWater;
    bool mCanDisarmTraps;
    bool mIsEvil;
    bool mIsImmuneToTurncoat;
    bool mAvailableViaPortal;
    bool mCanFly;
    bool mIsHorny;
    bool mLeavesCorpse;
    bool mCanBeHypnotized;
    bool mIsImmuneToChicken;
    bool mIsFearless;
    bool mCanBeElectrocuted;
    bool mNeedBodyForFightIdle;
    bool mNotTrainWhenIdle; //inverse value
    bool mOnlyAttackableByHorny;
    bool mCanBeResurrected;
    bool mDoesntGetAngryWithEnemies;
    bool mFreesFriendsOnJailbreak;
    bool mRevealsAdjacentTraps;
    bool mIsUnique;
    bool mIsMale;
    bool mIsImmuneToLightning {};
    bool mIsStoneKnight {};
    bool mIsEmotionless {};
    bool mAvailableViaHeroPortal {};
};

//////////////////////////////////////////////////////////////////////////

struct ScenarioObjectThing
{
public:
    GameObjectClassId mObjectClassId = {};
    int mPositionX = 0;
    int mPositionY = 0;
    ePlayerID mPlayerId = {};
    int mKeeperSpellId = 0;
    int mMoneyAmount = 0;
    int mTriggerId = 0;
};

//////////////////////////////////////////////////////////////////////////

struct ScenarioRoomThing
{
public:
    RoomTypeId mRoomType = {};
    int mPositionX = 0;
    int mPositionY = 0;
    ePlayerID mPlayerId = {};
    int mInitialHealth = 0;
    eDirection mDirection = eDirection_N;
};

//////////////////////////////////////////////////////////////////////////

struct ScenarioCreatureThing
{
public:
    int mPositionX = 0;
    int mPositionY = 0;
    int mPositionZ = 0;// depth ?
    int mGoldHeld = 0; // percents
    int mLevel = 1;
    int mInitialHealth = 0;  // percents
    ePlayerID mObjectiveTargetPlayerId {};
    ePlayerID mPlayerId {};
    CreatureTypeId mCreatureTypeId {};

    struct 
    {
        bool mWillFight {};
        bool mIsLeader {};
        bool mIsFollower {};
        bool mWillBeAttacked {};
        bool mReturnToHeroLair {};
        bool mFreeFriendsOnJailBreak {};
        bool mActAsDropped {};
        bool mStartAsDying {};
    }
    mBehaviorFlags;
};

//////////////////////////////////////////////////////////////////////////

struct ScenarioLevelFlags
{
public:
    bool mAlwaysImprisonEnemies {};
    bool mOneShotHornySpellAvailable {};
    bool mIsSecretLevel {};
    bool mIsSpecialLevel {};
    bool mDisplayHeroesKilledTally {};
    bool mAutomaticallyShowObjectiveBox {};
    bool mLastHeartGeneratesPortalGem {};
    bool mFreezeGameOptions {};
    bool mIsMultiplayerLevel {};
    bool mIsSkirmishLevel {};
    bool mIsMyPetDungeonLevel {};
};

//////////////////////////////////////////////////////////////////////////

struct ScenarioLevelInfo
{
public:
    std::string mFileName;
    std::wstring mLevelName;
    std::wstring mDescription;
    std::wstring mAuthorName;
    std::wstring mEmailAddress;
    std::wstring mInformation;
    ScenarioLevelFlags mFlags {};
    TextTableId mTextTableId = TextTableId_Null;
    TextTableId mBriefingTableId = TextTableId_Null;
    int mMapDimsX {}; // width in tiles
    int mMapDimsY {}; // height in tiles
    int mMaxPlayerCount = 1;
    float mTicksPerSecond = 1.0f;
};

//////////////////////////////////////////////////////////////////////////
// Contains all information about level and world
//////////////////////////////////////////////////////////////////////////

struct ScenarioDefinition
{
public:
    ScenarioDefinition()
        : mLevelInfo()
        , mLavaTerrainType(TerrainTypeId_Null)
        , mWaterTerrainType(TerrainTypeId_Null)
        , mPlayerColouredPathTerrainType(TerrainTypeId_Null)
        , mPlayerColouredWallTerrainType(TerrainTypeId_Null)
        , mFogOfWarTerrainType(TerrainTypeId_Null)
    {
    }

    // Find object class definition by its name
    // @param className: Object class
    inline GameObjectDefinition* GetObjectDefinition(const std::string& className) const
    {
        for (const GameObjectDefinition& currentDefinition: mGameObjectDefs)
        {
            if (currentDefinition.mObjectName == className)
                return const_cast<GameObjectDefinition*>(&currentDefinition);
        }
        return nullptr;
    }

    inline GameObjectDefinition* GetObjectDefinition(GameObjectClassId classId) const
    {
        if ((classId > 0) && (classId < mGameObjectDefs.size()))
        {
            return const_cast<GameObjectDefinition*>(&mGameObjectDefs[classId]);
        }
        return nullptr;
    }

    // Find room class definition by its name
    // @param className: Room class
    inline RoomDefinition* GetRoomDefinition(const std::string& className) const
    {
        for (const RoomDefinition& currentDefinition: mRoomDefs)
        {
            if (currentDefinition.mRoomName == className)
                return const_cast<RoomDefinition*>(&currentDefinition);
        }
        return nullptr;
    }

    inline RoomDefinition* GetRoomDefinition(RoomTypeId classId) const
    {
        if ((classId > 0) && (classId < mRoomDefs.size()))
        {
            return const_cast<RoomDefinition*>(&mRoomDefs[classId]);
        }
        return nullptr;
    }

    // Find creature class definition by its name
    // @param className: Creature class
    inline CreatureDefinition* GetCreatureDefinition(const std::string& className) const
    {
        for (const CreatureDefinition& currentDefinition: mCreatureDefs)
        {
            if (currentDefinition.mCreatureName == className)
                return const_cast<CreatureDefinition*>(&currentDefinition);
        }
        return nullptr;
    }

    inline CreatureDefinition* GetCreatureDefinition(CreatureTypeId creatureTypeId) const
    {
        if ((creatureTypeId > 0) && (creatureTypeId < mCreatureDefs.size()))
        {
            return const_cast<CreatureDefinition*>(&mCreatureDefs[creatureTypeId]);
        }
        return nullptr;
    }

    // Get room definition by corresponding terrain identifier
    inline TerrainDefinition* GetTerrainDefinition(TerrainTypeId terrainTypeId) const
    {
        if ((terrainTypeId > 0) && (terrainTypeId < mTerrainDefs.size()))
        {
            return const_cast<TerrainDefinition*>(&mTerrainDefs[terrainTypeId]);
        }
        return nullptr;
    }

    // Get room definition by corresponding terrain identifier
    inline RoomDefinition* GetRoomDefinitionByTerrain(TerrainDefinition* terrainDefinition) const
    {
        cxx_assert(terrainDefinition);
        return GetRoomDefinitionByTerrain(terrainDefinition->mTerrainType);
    }

    // Get room definition by corresponding terrain identifier
    inline RoomDefinition* GetRoomDefinitionByTerrain(TerrainTypeId terrainTypeId) const
    {
        const RoomTypeId roomTypeID = mRoomByTerrainType[terrainTypeId];
        return const_cast<RoomDefinition*>(&mRoomDefs[roomTypeID]);
    }

    // get player definition by identifier
    inline PlayerDefinition* GetPlayerDefinition(ePlayerID playerId) const
    {
        if ((playerId > 0) && (playerId < mPlayerDefs.size()))
        {
            return const_cast<PlayerDefinition*>(&mPlayerDefs[playerId]);
        }
        return nullptr;
    }

    // Test whether terrain is room
    inline bool IsRoomTypeTerrain(TerrainDefinition* terrainDefinition) const
    {
        cxx_assert(terrainDefinition);
        return IsRoomTypeTerrain(terrainDefinition->mTerrainType);
    }

    // Test whether terrain is room
    inline bool IsRoomTypeTerrain(TerrainTypeId terrainTypeId) const 
    {
        return (mRoomByTerrainType[terrainTypeId] != RoomTypeId_Null);
    }

public:
    ScenarioLevelInfo mLevelInfo;
    ScenarioVariables mVariables;
    TerrainTypeId mLavaTerrainType;
    TerrainTypeId mWaterTerrainType;
    TerrainTypeId mPlayerColouredPathTerrainType;
    TerrainTypeId mPlayerColouredWallTerrainType;
    TerrainTypeId mFogOfWarTerrainType;
    std::vector<PlayerDefinition> mPlayerDefs; // first entry in definition list is dummy
    std::vector<TerrainDefinition> mTerrainDefs; // first entry in definition list is dummy
    std::vector<RoomDefinition> mRoomDefs; // first entry in definition list is dummy
    std::vector<GameObjectDefinition> mGameObjectDefs; // first entry in definition list is dummy
    std::vector<RoomTypeId> mRoomByTerrainType; // map room types to terrain types
    std::vector<ScenarioMapTileDefinition> mMapTiles; // tiles matrix
    std::vector<CreatureDefinition> mCreatureDefs;
    // things
    std::vector<ScenarioObjectThing> mObjectThings;
    std::vector<ScenarioRoomThing> mRoomThings;
    std::vector<ScenarioCreatureThing> mCreatureThings;
};