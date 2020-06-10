#pragma once

// scenario art resource type
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

decl_enum_strings(eArtResource);

// scenario art resource data
struct ArtResource 
{
public:
    ArtResource()
        : mStartAF()
        , mEndAF()
        , mResourceType(eArtResource_Null)
        , mPlayerColoured()
        , mAnimatingTexture()
        , mHasStartAnimation()
        , mHasEndAnimation()
        , mRandomStartFrame()
        , mOriginAtBottom()
        , mDoesntLoop()
        , mFlat()
        , mDoesntUseProgressiveMesh()
        , mUseAnimatingTextureForSelection()
        , mPreload()
        , mBlood()
    {
    }

    // test whether resource is defined
    inline bool IsDefined() const { return mResourceType > eArtResource_Null; }

public:

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

public:
    eArtResource mResourceType;
    std::string mResourceName;
    int mStartAF;
    int mEndAF;
    bool mPlayerColoured;
    bool mAnimatingTexture;
    bool mHasStartAnimation;
    bool mHasEndAnimation;
    bool mRandomStartFrame;
    bool mOriginAtBottom;
    bool mDoesntLoop;
    bool mFlat;
    bool mDoesntUseProgressiveMesh;
    bool mUseAnimatingTextureForSelection;
    bool mPreload;
    bool mBlood;
};

// data
struct ComputerPlayerPreferences
{

};
    
// player definition
struct PlayerDefinition
{
public:
    PlayerDefinition()
        : mPlayerIdentifier(ePlayerID_Null)
        , mPlayerType(ePlayerType_Dummy)
        , mInitialGold()
        , mInitialMana()
        , mManaValue()
        , mGoldValue()
        , mStartCameraX()
        , mStartCameraY()
        , mComputerAI(eComputerAI_MasterKeeper)
    {
    }

public:
    ePlayerID mPlayerIdentifier;
    ePlayerType mPlayerType;
    std::string mPlayerName;
    int mStartCameraX;
    int mStartCameraY;
    int mInitialGold;
    int mInitialMana;
    int mManaValue;
    int mGoldValue;
    eComputerAI mComputerAI;
    ComputerPlayerPreferences mAIPreferences;
};

// map tile definition

// terrain under the bridge
enum eBridgeTerrain 
{
    eBridgeTerrain_Null,
    eBridgeTerrain_Water,
    eBridgeTerrain_Lava,
    eBridgeTerrain_COUNT
};

decl_enum_strings(eBridgeTerrain);

// single map tile information
struct MapTileDefinition
{
public:
    TerrainTypeID mTerrainType = TerrainType_Null;
    ePlayerID mOwnerIdentifier; // owner
    eBridgeTerrain mTerrainUnderTheBridge = eBridgeTerrain_Null;
};

// terrain Type Definition
struct TerrainDefinition 
{
public:
    TerrainDefinition()
        : mTerrainType(TerrainType_Null)
        , mBecomesTerrainTypeWhenMaxHealth(TerrainType_Null)
        , mBecomesTerrainTypeWhenDestroyed(TerrainType_Null)
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
        , mGoldCapacity()
        , mManaGain()
        , mManaGainMax()
        , mHealthInitial()
        , mHealthMax()
        , mAmbientColor(Color32_White)
        , mTerrainColor(Color32_White)
        , mLightHeight()
    {
    }

    // get cell resource shortcut
    inline ArtResource* GetCellResource() 
    {
        ArtResource& cellResource = (mIsSolid && !mConstructionTypeQuad) ? mResourceTop : mResourceComplete;
        debug_assert(cellResource.IsDefined());
        return &cellResource;
    }

public:
    TerrainTypeID mTerrainType;
    std::string mName;
    TerrainTypeID mBecomesTerrainTypeWhenMaxHealth;
    TerrainTypeID mBecomesTerrainTypeWhenDestroyed;
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
    ArtResource mResourceComplete;
    ArtResource mResourceSide;
    ArtResource mResourceTop;
    ArtResource mResourceTagged;
    int mTextureFrames;
    int mDamage;
    int mGoldCapacity;
    int mManaGain;
    int mManaGainMax;
    int mHealthInitial;
    int mHealthMax;
    float mLightHeight;
    Color32 mTerrainColor;
    Color32 mAmbientColor;
};

// room Type Definition
struct RoomDefinition
{
public:
    RoomDefinition()
        : mRoomType(RoomType_Null)
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
        , mTerrainType(TerrainType_Null)
        , mCost()
        , mOrderInEditor()
        , mFloorObjectsIds()
        , mWallObjectsIds()
    {
    }

    inline bool HandlesWalls() const
    {
        // hero gate disables default terrain walls construction
        return mHasWalls || mTileConstruction == eRoomTileConstruction_HeroGateFrontEnd ||
            mTileConstruction == eRoomTileConstruction_HeroGate_3_by_1;
    }

public:
    RoomTypeID mRoomType;
    std::string mRoomName;
    ArtResource mGuiIconResource;
    ArtResource mEditorIconResource;
    ArtResource mCompleteResource;
    ArtResource mStraightResource;
    ArtResource mInsideCornerResource;
    ArtResource mUnknownResource;
    ArtResource mOutsideCornerResource;
    ArtResource mWallResource;
    ArtResource mCapResource;
    ArtResource mCeilingResource;
    ArtResource mTorchResource;
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
    int mFloorObjectsIds[4];
    int mWallObjectsIds[4];
    TerrainTypeID mTerrainType;
    eRoomTileConstruction mTileConstruction;
    std::string mSoundCategory;
    int mRecommendedSizeX;
    int mRecommendedSizeY;
};

// object Type Definition
struct GameObjectDefinition
{
public:
    GameObjectDefinition() 
        : mObjectType(GameObjectType_Null)
        , mDieOverTime()
        , mDieOverTimeIfNotInRoom()
        , mTypeSpecial()
        , mTypeSpellBook()
        , mTypeCrate()  
        , mTypeLair()
        , mTypeGold()
        , mTypeFood()
        , mCanBePickedUp()
        , mCanBeSlapped()            
        , mDieWhenSlapped()
        , mTypeLevelGem()
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
        , mObjectMaterial(eGameObjectMaterial_None)
        , mTooltipStringId()
        , mNameStringId()
        , mSlapEffectId()
        , mDeathEffectId()
        , mMiscEffectId()
    {
    }

public:
    GameObjectTypeID mObjectType;
    std::string mObjectName; 
    ArtResource mMeshResource; 
    ArtResource mGuiIconResource;
    ArtResource mInHandIconResource; 
    ArtResource mInHandMeshResource;
    ArtResource mUnknownResource;
    float mWidth;
    float mHeight;
    float mPhysicsMass;
    float mPhysicsSpeed;
    float mPhysicsAirFriction;
    int mHitpoints;
    int mMaxAngle;
    int mManaValue;
    int mRoomCapacity;
    int mTooltipStringId;
    int mNameStringId;
    int mSlapEffectId;
    int mDeathEffectId;
    int mMiscEffectId;
    eGameObjectMaterial mObjectMaterial;
    bool mDieOverTime;
    bool mDieOverTimeIfNotInRoom;
    bool mTypeSpecial;
    bool mTypeSpellBook;
    bool mTypeCrate;  
    bool mTypeLair;
    bool mTypeGold;
    bool mTypeFood;
    bool mCanBePickedUp;
    bool mCanBeSlapped;             
    bool mDieWhenSlapped;
    bool mTypeLevelGem;
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

// scenario variables
struct ScenarioVariables
{
public:
    // todo

    inline void Clear()
    {
    }
};

// defines creature properties
class CreatureDefinition
{
public:
    CreatureDefinition()
        : mCreatureClass(CreatureType_Null)
        , mCloneCreatureId(CreatureType_Null)
    {
    }

public:
    CreatureTypeID mCreatureClass;
    std::string mCreatureName;
	ArtResource mAnimationResources[eCreatureAnimation_COUNT];
    ArtResource mIcon1Resource;
    ArtResource mIcon2Resource;
    ArtResource mPortraitResource;
    ArtResource mFirstPersonFilterResource;
    ArtResource mFirstPersonMeleeResource;
    ArtResource mUniqueResource;
    int mOrderInEditor;
    int mAngerStringIdGeneral;
    int mShotDelay;
    int mOlhiEffectId;
    int mIntroductionStringId;
    int mPerceptionRange;
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
    CreatureTypeID mCloneCreatureId;
    glm::vec3 mAnimationOffset;
    glm::vec3 mAnimationOffsets[7];
    eCreatureJobClass mCreatureJobClass;
    eGameObjectMaterial mCreatureArmour;
    bool mIsWorker;
    bool mCanBePickedUp;
    bool mCanBeSlapped;
    bool mAlwaysFlee;
    bool mCanWalkOnLava;
    bool mCanWalkOnWater;
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
    bool mCameraRollsWhenTurning;
    bool mIsMale;
    bool mIsImmuneToLightning;
    bool mIsStoneKnight;
    bool mIsEmotionless;
    bool mAvailableViaHeroPortal;
};

//////////////////////////////////////////////////////////////////////////
// Contains all information about level and world
//////////////////////////////////////////////////////////////////////////

struct ScenarioData
{
public:
    ScenarioData()
        : mLevelDimensionX()
        , mLevelDimensionY()
        , mLavaTerrainType(TerrainType_Null)
        , mWaterTerrainType(TerrainType_Null)
        , mPlayerColouredPathTerrainType(TerrainType_Null)
        , mPlayerColouredWallTerrainType(TerrainType_Null)
        , mFogOfWarTerrainType(TerrainType_Null)
    {
    }

    inline void Clear()
    {
        mLevelName.clear();
        mLevelDescription.clear();
        mLevelAuthor.clear();
        mLevelEmail.clear();
        mLevelInformation.clear();

        mLevelDimensionX = 0;
        mLevelDimensionY = 0;

        mVariables.Clear();

        mLavaTerrainType                = TerrainType_Null;
        mWaterTerrainType               = TerrainType_Null;
        mPlayerColouredPathTerrainType  = TerrainType_Null;
        mPlayerColouredWallTerrainType  = TerrainType_Null;
        mFogOfWarTerrainType            = TerrainType_Null;

        mPlayerDefs.clear();
        mTerrainDefs.clear();
        mRoomDefs.clear();
        mGameObjectDefs.clear();
        mRoomByTerrainType.clear();
        mMapTiles.clear();
        mCreatureDefs.clear();
    }

public:
    std::string mLevelName;
    std::string mLevelDescription;
    std::string mLevelAuthor;
    std::string mLevelEmail;
    std::string mLevelInformation;
    int mLevelDimensionX; // width in tiles
    int mLevelDimensionY; // height in tiles
    ScenarioVariables mVariables;
    TerrainTypeID mLavaTerrainType;
    TerrainTypeID mWaterTerrainType;
    TerrainTypeID mPlayerColouredPathTerrainType;
    TerrainTypeID mPlayerColouredWallTerrainType;
    TerrainTypeID mFogOfWarTerrainType;
    std::vector<PlayerDefinition> mPlayerDefs; // first entry is dummy
    std::vector<TerrainDefinition> mTerrainDefs; // first entry is dummy
    std::vector<RoomDefinition> mRoomDefs; // first entry is dummy
    std::vector<GameObjectDefinition> mGameObjectDefs; // first entry is dummy
    std::vector<RoomTypeID> mRoomByTerrainType; // map room types to terrain types
    std::vector<MapTileDefinition> mMapTiles; // tiles matrix
    std::vector<CreatureDefinition> mCreatureDefs; // first entry is dummy
};