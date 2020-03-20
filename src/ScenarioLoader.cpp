#include "pch.h"
#include "ScenarioLoader.h"
#include "Console.h"
#include "FileSystem.h"

#define DIVIDER_FLOAT 4096.0f
#define DIVIDER_DOUBLE 65536.0f

#define READ_FSTREAM_DATATYPE(filestream, destination, datatype) \
    { \
        datatype dataBuffer; \
        long bytesRead = filestream->ReadData(&dataBuffer, sizeof(datatype)); \
        if (bytesRead != sizeof(datatype)) \
        { \
            return false; \
        } \
        destination = static_cast<std::remove_reference<decltype(destination)>::type>(dataBuffer); \
    }

#define SKIP_FSTREAM_BYTES(filestream, numBytes) \
    { \
        if (!filestream->AdvanceCursorPosition(numBytes)) \
        { \
            return false; \
        } \
    }

#define READ_FSTREAM_UINT8(filestream, destination) \
    READ_FSTREAM_DATATYPE(filestream, destination, unsigned char)

#define READ_FSTREAM_UINT16(filestream, destination) \
    READ_FSTREAM_DATATYPE(filestream, destination, unsigned short)

#define READ_FSTREAM_UINT32(filestream, destination) \
    READ_FSTREAM_DATATYPE(filestream, destination, unsigned int)

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

inline bool KwdToENUM(unsigned int inputInt, ePlayerID& outputID)
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
    if (inputInt < ePlayerID_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KwdToENUM(unsigned int inputInt, eBridgeTerrain& outputID)
{
    static const eBridgeTerrain IDs[] =
    {
        eBridgeTerrain_Null, // 0
        eBridgeTerrain_Water, // 1
        eBridgeTerrain_Lava, // 2
    };
    if (inputInt < eBridgeTerrain_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KwdToENUM(unsigned int inputInt, eGameObjectMaterial& outputID)
{
    static const eGameObjectMaterial IDs[] =
    {
        eGameObjectMaterial_None, // 0
        eGameObjectMaterial_Flesh, // 1
        eGameObjectMaterial_Rock, // 2
        eGameObjectMaterial_Wood, // 3
        eGameObjectMaterial_Metal1, // 4
        eGameObjectMaterial_Metal2, // 5
        eGameObjectMaterial_Magic, // 6
        eGameObjectMaterial_Glass, // 7
    };
    if (inputInt < eGameObjectMaterial_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KwdToENUM(unsigned int inputInt, eArtResource& outputID)
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
    if (inputInt < eArtResource_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KwdToENUM(unsigned int inputInt, ePlayerType& outputID)
{
    static const ePlayerType IDs[] =
    {
        ePlayerType_Human, // 0
        ePlayerType_Computer, // 1
    };
    if (inputInt < ePlayerType_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KwdToENUM(unsigned int inputInt, eComputerAI& outputID)
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
    if (inputInt < eComputerAI_COUNT)
    {
        outputID = IDs[inputInt];
        return true;
    }
    return false;
}

inline bool KwdToENUM(unsigned int inputInt, eRoomTileConstruction& outputID)
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
        eRoomTileConstruction_HeroGateFrontEnd,
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

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

bool ScenarioLoader::ReadString(BinaryInputStream* fileStream, unsigned int stringLength, std::string& ansiString)
{
    const long dataLength = stringLength * 2;

    ByteArray bytes;
    bytes.resize(dataLength);

    // read bytes
    long readBytes = fileStream->ReadData(bytes.data(), dataLength);
    if (readBytes != dataLength)
        return false;

    // convert
    std::wstring tempString(reinterpret_cast<wchar_t*>(&bytes[0]), stringLength);
    ansiString.assign(
        tempString.begin(), 
        tempString.end());

    // trim
    ansiString.erase(
        std::find_if(ansiString.begin(), ansiString.end(), [](const char& c)->bool {
            return c == 0;
        }), 
        ansiString.end());

    return true;
}

bool ScenarioLoader::ReadString8(BinaryInputStream* fileStream, unsigned int stringLength, std::string& ansiString)
{
    ByteArray bytes;
    bytes.resize(stringLength + 1);

    // read bytes
    long readBytes = fileStream->ReadData(bytes.data(), stringLength);
    if (readBytes != stringLength)
        return false;

    ansiString.assign(bytes.begin(), bytes.end());

    // trim
    ansiString.erase(
        std::find_if(ansiString.begin(), ansiString.end(), [](const char& c)->bool {
            return c == 0;
        }), 
        ansiString.end());

    return true;
}

bool ScenarioLoader::ReadTimestamp(BinaryInputStream* fileStream)
{
    unsigned int ignoreData;
    READ_FSTREAM_UINT16(fileStream, ignoreData); // year
    READ_FSTREAM_UINT8(fileStream, ignoreData); // day
    READ_FSTREAM_UINT8(fileStream, ignoreData); // month
    READ_FSTREAM_UINT16(fileStream, ignoreData); // unknown
    READ_FSTREAM_UINT8(fileStream, ignoreData); // hour
    READ_FSTREAM_UINT8(fileStream, ignoreData); // minute
    READ_FSTREAM_UINT8(fileStream, ignoreData); // second
    READ_FSTREAM_UINT8(fileStream, ignoreData); // unknown
    return true;
}

bool ScenarioLoader::Read32bitsFloat(BinaryInputStream* fileStream, float& outputFloat)
{
    unsigned int encoded_float;
    READ_FSTREAM_UINT32(fileStream, encoded_float);

    // decode float
    outputFloat = encoded_float / DIVIDER_FLOAT;
    return true;
}

bool ScenarioLoader::ReadLight(BinaryInputStream* fileStream)
{
    unsigned int ikpos;
    READ_FSTREAM_UINT32(fileStream, ikpos); // x, / ConversionUtils.FLOAT
    READ_FSTREAM_UINT32(fileStream, ikpos); // y, / ConversionUtils.FLOAT
    READ_FSTREAM_UINT32(fileStream, ikpos); // z, / ConversionUtils.FLOAT

    unsigned int iradius;
    READ_FSTREAM_UINT32(fileStream, iradius); // / ConversionUtils.FLOAT

    unsigned int flags;
    READ_FSTREAM_UINT32(fileStream, flags);

    unsigned char rgb;
    READ_FSTREAM_UINT8(fileStream, rgb); // r
    READ_FSTREAM_UINT8(fileStream, rgb); // g
    READ_FSTREAM_UINT8(fileStream, rgb); // b
    READ_FSTREAM_UINT8(fileStream, rgb); // a

    return true;
}

bool ScenarioLoader::ReadArtResource(BinaryInputStream* fileStream, ArtResource& artResource)
{
    if (!ReadString8(fileStream, 64, artResource.mResourceName))
        return false;

    unsigned int flags = 0;
    READ_FSTREAM_UINT32(fileStream, flags);

    // reading flags
    artResource.mPlayerColoured                     = (flags & ARTRESF_PLAYER_COLOURED) > 0;
    artResource.mAnimatingTexture                   = (flags & ARTRESF_ANIMATING_TEXTURE) > 0;
    artResource.mHasStartAnimation                  = (flags & ARTRESF_HAS_START_ANIMATION) > 0;
    artResource.mHasEndAnimation                    = (flags & ARTRESF_HAS_END_ANIMATION) > 0;
    artResource.mRandomStartFrame                   = (flags & ARTRESF_RANDOM_START_FRAME) > 0;
    artResource.mOriginAtBottom                     = (flags & ARTRESF_ORIGIN_AT_BOTTOM) > 0;
    artResource.mDoesntLoop                         = (flags & ARTRESF_DOESNT_LOOP) > 0;
    artResource.mFlat                               = (flags & ARTRESF_FLAT) > 0;
    artResource.mDoesntUseProgressiveMesh           = (flags & ARTRESF_DOESNT_USE_PROGRESSIVE_MESH) > 0;
    artResource.mUseAnimatingTextureForSelection    = (flags & ARTRESF_USE_ANIMATING_TEXTURE_FOR_SELECTION) > 0;
    artResource.mPreload                            = (flags & ARTRESF_PRELOAD) > 0;
    artResource.mBlood                              = (flags & ARTRESF_BLOOD) > 0;

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

    const long packed_data_length = 12;

    static_assert(sizeof(packed_data) == packed_data_length, "Wrong size");

    long bytesRead = fileStream->ReadData(&packed_data, packed_data_length);
    if (bytesRead != packed_data_length)
        return false;

    // map resource type
    unsigned char resourceType;
    READ_FSTREAM_UINT8(fileStream, resourceType);
    if (!KwdToENUM(resourceType, artResource.mResourceType))
        return false;

    READ_FSTREAM_UINT8(fileStream, artResource.mStartAF);
    READ_FSTREAM_UINT8(fileStream, artResource.mEndAF);

    unsigned char unknownByte;
    READ_FSTREAM_UINT8(fileStream, unknownByte);

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

bool ScenarioLoader::ReadTerrainFlags(BinaryInputStream* fileStream, TerrainDefinition& terrainDef)
{
    unsigned int flags = 0;
    READ_FSTREAM_UINT32(fileStream, flags);

    terrainDef.mIsSolid                         = (flags & TERRAINF_SOLID) > 0;
    terrainDef.mIsImpenetrable                  = (flags & TERRAINF_IMPENETRABLE) > 0;
    terrainDef.mIsOwnable                       = (flags & TERRAINF_OWNABLE) > 0;
    terrainDef.mIsTaggable                      = (flags & TERRAINF_TAGGABLE) > 0;
    terrainDef.mIsAttackable                    = (flags & TERRAINF_ATTACKABLE) > 0;
    terrainDef.mHasTorch                        = (flags & TERRAINF_TORCH) > 0;
    terrainDef.mIsWater                         = (flags & TERRAINF_WATER) > 0;
    terrainDef.mIsLava                          = (flags & TERRAINF_LAVA) > 0;
    terrainDef.mAlwaysExplored                  = (flags & TERRAINF_ALWAYS_EXPLORED) > 0;
    terrainDef.mPlayerColouredPath              = (flags & TERRAINF_PLAYER_COLOURED_PATH) > 0;
    terrainDef.mPlayerColouredWall              = (flags & TERRAINF_PLAYER_COLOURED_WALL) > 0;
    terrainDef.mConstructionTypeWater           = (flags & TERRAINF_CONSTRUCTION_TYPE_WATER) > 0;
    terrainDef.mConstructionTypeQuad            = (flags & TERRAINF_CONSTRUCTION_TYPE_QUAD) > 0;
    terrainDef.mUnexploreIfDugByAnotherPlayer   = (flags & TERRAINF_UNEXPLORE_IF_DUG_BY_ANOTHER_PLAYER) > 0;
    terrainDef.mFillInable                      = (flags & TERRAINF_FILL_INABLE) > 0;
    terrainDef.mAllowRoomWalls                  = (flags & TERRAINF_ALLOW_ROOM_WALLS) > 0;
    terrainDef.mIsDecay                         = (flags & TERRAINF_DECAY) > 0;
    terrainDef.mHasRandomTexture                = (flags & TERRAINF_RANDOM_TEXTURE) > 0;
    terrainDef.mTerrainColorR                   = (flags & TERRAINF_TERRAIN_COLOR_R) > 0;
    terrainDef.mTerrainColorG                   = (flags & TERRAINF_TERRAIN_COLOR_G) > 0;
    terrainDef.mTerrainColorB                   = (flags & TERRAINF_TERRAIN_COLOR_B) > 0;
    terrainDef.mDwarfCanDigThrough              = (flags & TERRAINF_DWARF_CAN_DIG_THROUGH) > 0;
    terrainDef.mRevealThroughFogOfWar           = (flags & TERRAINF_REVEAL_THROUGH_FOG_OF_WAR) > 0;
    terrainDef.mAmbientColorR                   = (flags & TERRAINF_AMBIENT_COLOR_R) > 0;
    terrainDef.mAmbientColorG                   = (flags & TERRAINF_AMBIENT_COLOR_G) > 0;
    terrainDef.mAmbientColorB                   = (flags & TERRAINF_AMBIENT_COLOR_B) > 0;
    terrainDef.mHasLight                        = (flags & TERRAINF_LIGHT) > 0;
    terrainDef.mHasAmbientLight                 = (flags & TERRAINF_AMBIENT_LIGHT) > 0;

    return true;
}

bool ScenarioLoader::ReadRoomFlags(BinaryInputStream* fileStream, RoomDefinition& roomDef)
{
    unsigned int flags = 0;
    READ_FSTREAM_UINT32(fileStream, flags);

    roomDef.mPlaceableOnWater   = (flags & ROOMF_PLACEABLE_ON_WATER) > 0;
    roomDef.mPlaceableOnLava    = (flags & ROOMF_PLACEABLE_ON_LAVA) > 0;
    roomDef.mPlaceableOnLand    = (flags & ROOMF_PLACEABLE_ON_LAND) > 0;
    roomDef.mHasWalls           = (flags & ROOMF_HAS_WALLS) > 0;
    roomDef.mCentre             = (flags & ROOMF_CENTRE) > 0;
    roomDef.mSpecialTiles       = (flags & ROOMF_SPECIAL_TILES) > 0;
    roomDef.mNormalTiles        = (flags & ROOMF_NORMAL_TILES) > 0;
    roomDef.mBuildable          = (flags & ROOMF_BUILDABLE) > 0;
    roomDef.mSpecialWalls       = (flags & ROOMF_SPECIAL_WALLS) > 0;
    roomDef.mIsAttackable       = (flags & ROOMF_ATTACKABLE) > 0;
    roomDef.mHasFlame           = (flags & ROOMF_HAS_FLAME) > 0;
    roomDef.mIsGood             = (flags & ROOMF_IS_GOOD) > 0;

    return true;
}

bool ScenarioLoader::ReadObjectFlags(BinaryInputStream* fileStream, GameObjectDefinition& objectDef)
{
    unsigned int flags;
    READ_FSTREAM_UINT32(fileStream, flags);

    objectDef.mDieOverTime              = (flags & OBJECTF_DIE_OVER_TIME) > 0;
    objectDef.mDieOverTimeIfNotInRoom   = (flags & OBJECTF_DIE_OVER_TIME_IF_NOT_IN_ROOM) > 0;
    objectDef.mTypeSpecial              = (flags & OBJECTF_TYPE_SPECIAL) > 0;
    objectDef.mTypeSpellBook            = (flags & OBJECTF_TYPE_SPELL_BOOK) > 0;
    objectDef.mTypeCrate                = (flags & OBJECTF_TYPE_CRATE) > 0;
    objectDef.mTypeLair                 = (flags & OBJECTF_TYPE_LAIR) > 0;
    objectDef.mTypeGold                 = (flags & OBJECTF_TYPE_GOLD) > 0;
    objectDef.mTypeFood                 = (flags & OBJECTF_TYPE_FOOD) > 0;
    objectDef.mCanBePickedUp            = (flags & OBJECTF_CAN_BE_PICKED_UP) > 0;
    objectDef.mCanBeSlapped             = (flags & OBJECTF_CAN_BE_SLAPPED) > 0;
    objectDef.mDieWhenSlapped           = (flags & OBJECTF_DIE_WHEN_SLAPPED) > 0;
    objectDef.mTypeLevelGem             = (flags & OBJECTF_TYPE_LEVEL_GEM) > 0;
    objectDef.mCanBeDroppedOnAnyLand    = (flags & OBJECTF_CAN_BE_DROPPED_ON_ANY_LAND) > 0;
    objectDef.mObstacle                 = (flags & OBJECTF_OBSTACLE) > 0;
    objectDef.mBounce                   = (flags & OBJECTF_BOUNCE) > 0;
    objectDef.mBoulderCanRollThrough    = (flags & OBJECTF_BOULDER_CAN_ROLL_THROUGH) > 0;
    objectDef.mBoulderDestroys          = (flags & OBJECTF_BOULDER_DESTROYS) > 0;
    objectDef.mIsPillar                 = (flags & OBJECTF_PILLAR) > 0;
    objectDef.mDoorKey                  = (flags & OBJECTF_DOOR_KEY) > 0;
    objectDef.mIsDamageable             = (flags & OBJECTF_DAMAGEABLE) > 0;
    objectDef.mHighlightable            = (flags & OBJECTF_HIGHLIGHTABLE) > 0;
    objectDef.mPlaceable                = (flags & OBJECTF_PLACEABLE) > 0;
    objectDef.mFirstPersonObstacle      = (flags & OBJECTF_FIRST_PERSON_OBSTACLE) > 0;
    objectDef.mSolidObstacle            = (flags & OBJECTF_SOLID_OBSTACLE) > 0;
    objectDef.mCastShadows              = (flags & OBJECTF_CAST_SHADOWS) > 0;

    return true;
}

bool ScenarioLoader::ReadStringId(BinaryInputStream* fileStream)
{
    unsigned int ids[5];
    for (unsigned int& id_entry : ids)
    {
        READ_FSTREAM_UINT32(fileStream, id_entry);
    }

    unsigned int unknownDword;
    READ_FSTREAM_UINT32(fileStream, unknownDword);

    return true;
}

bool ScenarioLoader::ReadMapData(BinaryInputStream* fileStream)
{
    mScenarioData.mMapTiles.resize(mScenarioData.mLevelDimensionX * mScenarioData.mLevelDimensionY);

    // read tiles
    for (int tiley = 0; tiley < mScenarioData.mLevelDimensionY; ++tiley)
    for (int tilex = 0; tilex < mScenarioData.mLevelDimensionX; ++tilex)
    {
        const int tileIndex = (tiley * mScenarioData.mLevelDimensionX) + tilex;

        // terrain type is not mapped to internal id so it can be red as is
        READ_FSTREAM_UINT8(fileStream, mScenarioData.mMapTiles[tileIndex].mTerrainType);

        unsigned char playerID;
        READ_FSTREAM_UINT8(fileStream, playerID);
        if (!KwdToENUM(playerID, mScenarioData.mMapTiles[tileIndex].mOwnerIdentifier))
            return false;

        unsigned char bridgeTerrain;
        READ_FSTREAM_UINT8(fileStream, bridgeTerrain);
        if (!KwdToENUM(bridgeTerrain, mScenarioData.mMapTiles[tileIndex].mTerrainUnderTheBridge))
            return false;

        unsigned char filler;
        READ_FSTREAM_UINT8(fileStream, filler);
    }

    return true;
}

bool ScenarioLoader::ReadObjectDefinition(BinaryInputStream* fileStream, GameObjectDefinition& objectDef)
{
    if (!ReadString8(fileStream, 32, objectDef.mObjectName))
        return false;

    // resources
    if (!ReadArtResource(fileStream, objectDef.mResourceMesh))
        return false;

    if (!ReadArtResource(fileStream, objectDef.mResourceGuiIcon))
        return false;

    if (!ReadArtResource(fileStream, objectDef.mResourceInHandIcon))
        return false;

    if (!ReadArtResource(fileStream, objectDef.mResourceInHandMesh))
        return false;

    if (!ReadArtResource(fileStream, objectDef.mResourceUnknown))
        return false;

    ArtResource additionalResources[4]; // not used
    for (ArtResource& additionalResource : additionalResources)
    {
        if (!ReadArtResource(fileStream, objectDef.mResourceUnknown))
            return false;
    }

    if (!ReadLight(fileStream))
        return false;

    unsigned int width;
    unsigned int height;
    unsigned int mass;
    unsigned int speed;
    unsigned int airFriction;

    READ_FSTREAM_UINT32(fileStream, width);
    READ_FSTREAM_UINT32(fileStream, height);
    READ_FSTREAM_UINT32(fileStream, mass); 
    READ_FSTREAM_UINT32(fileStream, speed);
    READ_FSTREAM_UINT32(fileStream, airFriction);

    // set params
    objectDef.mWidth = (width * 1.0f) / DIVIDER_FLOAT;
    objectDef.mHeight = (height * 1.0f) / DIVIDER_FLOAT;
    objectDef.mPhysicsMass = (mass * 1.0f) / DIVIDER_FLOAT;
    objectDef.mPhysicsSpeed = (speed * 1.0f) / DIVIDER_FLOAT;
    objectDef.mPhysicsAirFriction = (airFriction * 1.0f) / DIVIDER_DOUBLE;

    unsigned char objMaterial;
    READ_FSTREAM_UINT8(fileStream, objMaterial);
    if (!KwdToENUM(objMaterial, objectDef.mObjectMaterial))
        return false;

    SKIP_FSTREAM_BYTES(fileStream, 3);

    if (!ReadObjectFlags(fileStream, objectDef))
        return false;

    READ_FSTREAM_UINT16(fileStream, objectDef.mHitpoints);
    READ_FSTREAM_UINT16(fileStream, objectDef.mMaxAngle);
    SKIP_FSTREAM_BYTES(fileStream, 2);
    READ_FSTREAM_UINT16(fileStream, objectDef.mManaValue);
    READ_FSTREAM_UINT16(fileStream, objectDef.mTooltipStringId);
    READ_FSTREAM_UINT16(fileStream, objectDef.mNameStringId);
    READ_FSTREAM_UINT16(fileStream, objectDef.mSlapEffectId);
    READ_FSTREAM_UINT16(fileStream, objectDef.mDeathEffectId);
    READ_FSTREAM_UINT16(fileStream, objectDef.mMiscEffectId);

    // object type is not mapped to internal id so it can be red as is
    READ_FSTREAM_UINT8(fileStream, objectDef.mObjectClass);

    unsigned char initialState;
    READ_FSTREAM_UINT8(fileStream, initialState);

    READ_FSTREAM_UINT8(fileStream, objectDef.mRoomCapacity);

    unsigned char pickupPriority;
    READ_FSTREAM_UINT8(fileStream, pickupPriority);

    // sound category
    if (!ReadString8(fileStream, 32, objectDef.mSoundCategory))
        return false;

    return true;
}

bool ScenarioLoader::ReadObjectsData(BinaryInputStream* fileStream, int numElements)
{
    mScenarioData.mGameObjectDefs.resize(numElements + 1);
    mScenarioData.mGameObjectDefs[0] = {}; // dummy element

    // read definitions
    for (int iobject = 1; iobject < numElements + 1; ++iobject)
    {
        if (!ReadObjectDefinition(fileStream, mScenarioData.mGameObjectDefs[iobject]))
            return false;

        bool correctId = (mScenarioData.mGameObjectDefs[iobject].mObjectClass == iobject);
        debug_assert(correctId);
    }
    return true;
}

bool ScenarioLoader::ReadPlayerDefinition(BinaryInputStream* fileStream, PlayerDefinition& playerDef)
{
    READ_FSTREAM_UINT32(fileStream, playerDef.mInitialGold);

    unsigned int playerType;
    READ_FSTREAM_UINT32(fileStream, playerType);
    if (!KwdToENUM(playerType, playerDef.mPlayerType))
        return false;

    unsigned char aiType;
    READ_FSTREAM_UINT8(fileStream, aiType);
    if (!KwdToENUM(aiType, playerDef.mComputerAI))
        return false;

    unsigned char speed;
    READ_FSTREAM_UINT8(fileStream, speed);

    unsigned char openness;
    READ_FSTREAM_UINT8(fileStream, openness);

    unsigned char fillerByte;
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    unsigned int fillerDword;
    READ_FSTREAM_UINT32(fileStream, fillerDword);
    READ_FSTREAM_UINT32(fileStream, fillerDword);
    READ_FSTREAM_UINT32(fileStream, fillerDword);

    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    READ_FSTREAM_UINT8(fileStream, fillerByte);

    // build order
    SKIP_FSTREAM_BYTES(fileStream, 15);

    READ_FSTREAM_UINT8(fileStream, fillerByte); // flexibility
    READ_FSTREAM_UINT8(fileStream, fillerByte); // digToNeutralRoomsWithinTilesOfClaimedArea
    
    unsigned short fillerWord;
    READ_FSTREAM_UINT16(fileStream, fillerWord); // removeCallToArmsAfterSeconds

    READ_FSTREAM_UINT32(fileStream, fillerDword); // boulderTrapsOnLongCorridors
    READ_FSTREAM_UINT32(fileStream, fillerDword); // boulderTrapsOnRouteToBreachPoints

    READ_FSTREAM_UINT8(fileStream, fillerByte); // trapUseStyle
    READ_FSTREAM_UINT8(fileStream, fillerByte); // doorTrapPreference

    READ_FSTREAM_UINT8(fileStream, fillerByte); // doorUsage
    READ_FSTREAM_UINT8(fileStream, fillerByte); // chanceOfLookingToUseTrapsAndDoors

    READ_FSTREAM_UINT32(fileStream, fillerDword); // requireMinLevelForCreatures
    READ_FSTREAM_UINT32(fileStream, fillerDword); // requireTotalThreatGreaterThanTheEnemy
    READ_FSTREAM_UINT32(fileStream, fillerDword); // requireAllRoomTypesPlaced
    READ_FSTREAM_UINT32(fileStream, fillerDword); // requireAllKeeperSpellsResearched
    READ_FSTREAM_UINT32(fileStream, fillerDword); // onlyAttackAttackers
    READ_FSTREAM_UINT32(fileStream, fillerDword); // neverAttack

    READ_FSTREAM_UINT8(fileStream, fillerByte); // minLevelForCreatures
    READ_FSTREAM_UINT8(fileStream, fillerByte); // totalThreatGreaterThanTheEnemy
    READ_FSTREAM_UINT8(fileStream, fillerByte); // firstAttemptToBreachRoom
    READ_FSTREAM_UINT8(fileStream, fillerByte); // firstDigToEnemyPoint
    READ_FSTREAM_UINT8(fileStream, fillerByte); // breachAtPointsSimultaneously
    READ_FSTREAM_UINT8(fileStream, fillerByte); // usePercentageOfTotalCreaturesInFirstFightAfterBreach

    READ_FSTREAM_UINT16(fileStream, playerDef.mManaValue); 
    READ_FSTREAM_UINT16(fileStream, fillerWord); // placeCallToArmsWhereThreatValueIsGreaterThan
    READ_FSTREAM_UINT16(fileStream, fillerWord); // removeCallToArmsIfLessThanEnemyCreatures

    READ_FSTREAM_UINT16(fileStream, fillerWord); // removeCallToArmsIfLessThanEnemyCreaturesWithinTiles
    READ_FSTREAM_UINT16(fileStream, fillerWord); // pullCreaturesFromFightIfOutnumberedAndUnableToDropReinforcements

    READ_FSTREAM_UINT8(fileStream, fillerByte); // threatValueOfDroppedCreaturesIsPercentageOfEnemyThreatValue
    READ_FSTREAM_UINT8(fileStream, fillerByte); // spellStyle
    READ_FSTREAM_UINT8(fileStream, fillerByte); // attemptToImprisonPercentageOfEnemyCreatures
    READ_FSTREAM_UINT8(fileStream, fillerByte); // ifCreatureHealthIsPercentageAndNotInOwnRoomMoveToLairOrTemple

    READ_FSTREAM_UINT16(fileStream, playerDef.mGoldValue);

    READ_FSTREAM_UINT32(fileStream, fillerDword); // tryToMakeUnhappyOnesHappy
    READ_FSTREAM_UINT32(fileStream, fillerDword); // tryToMakeAngryOnesHappy
    READ_FSTREAM_UINT32(fileStream, fillerDword); // disposeOfAngryCreatures
    READ_FSTREAM_UINT32(fileStream, fillerDword); // disposeOfRubbishCreaturesIfBetterOnesComeAlong

    READ_FSTREAM_UINT8(fileStream, fillerByte); // disposalMethod
    READ_FSTREAM_UINT8(fileStream, fillerByte); // maximumNumberOfImps
    READ_FSTREAM_UINT8(fileStream, fillerByte); // willNotSlapCreatures
    READ_FSTREAM_UINT8(fileStream, fillerByte); // attackWhenNumberOfCreaturesIsAtLeast

    READ_FSTREAM_UINT32(fileStream, fillerDword); // useLightningIfEnemyIsInWater

    READ_FSTREAM_UINT8(fileStream, fillerByte); // useSightOfEvil
    READ_FSTREAM_UINT8(fileStream, fillerByte); // useSpellsInBattle
    READ_FSTREAM_UINT8(fileStream, fillerByte); // spellsPowerPreference
    READ_FSTREAM_UINT8(fileStream, fillerByte); // useCallToArms
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    READ_FSTREAM_UINT16(fileStream, fillerWord); // mineGoldUntil
    READ_FSTREAM_UINT16(fileStream, fillerWord); // waitSecondsAfterPreviousAttackBeforeAttackingAgain

    READ_FSTREAM_UINT32(fileStream, playerDef.mInitialMana);

    READ_FSTREAM_UINT16(fileStream, fillerWord); // exploreUpToTilesToFindSpecials
    READ_FSTREAM_UINT16(fileStream, fillerWord); // impTilesRatio
    READ_FSTREAM_UINT16(fileStream, fillerWord); // buildStartX
    READ_FSTREAM_UINT16(fileStream, fillerWord); // buildStartY
    READ_FSTREAM_UINT16(fileStream, fillerWord); // buildEndX
    READ_FSTREAM_UINT16(fileStream, fillerWord); // buildEndY

    READ_FSTREAM_UINT8(fileStream, fillerByte); // likelyhoodToMovingCreaturesToLibraryForResearching
    READ_FSTREAM_UINT8(fileStream, fillerByte); // chanceOfExploringToFindSpecials
    READ_FSTREAM_UINT8(fileStream, fillerByte); // chanceOfFindingSpecialsWhenExploring
    READ_FSTREAM_UINT8(fileStream, fillerByte); // fateOfImprisonedCreatures

    READ_FSTREAM_UINT16(fileStream, fillerWord); // triggerId

    unsigned char playerID;
    READ_FSTREAM_UINT8(fileStream, playerID);
    if (!KwdToENUM(playerID, playerDef.mPlayerIdentifier))
        return false;

    READ_FSTREAM_UINT16(fileStream, playerDef.mStartCameraX); // cameraX
    READ_FSTREAM_UINT16(fileStream, playerDef.mStartCameraY); // cameraY

    if (!ReadString8(fileStream, 32, playerDef.mPlayerName))
        return false;

    return true;
}

bool ScenarioLoader::ReadPlayersData(BinaryInputStream* fileStream, int numElements)
{
    mScenarioData.mPlayerDefs.resize(numElements + 1);
    mScenarioData.mPlayerDefs[0] = {}; // dummy element

    // read definitions
    for (int iplayer = 1; iplayer < numElements + 1; ++iplayer)
    {
        if (!ReadPlayerDefinition(fileStream, mScenarioData.mPlayerDefs[iplayer]))
            return false;

        bool correctId = (mScenarioData.mPlayerDefs[iplayer].mPlayerIdentifier == iplayer);
        debug_assert(correctId);
    }
    return true;
}

bool ScenarioLoader::ReadRoomDefinition(BinaryInputStream* fileStream, RoomDefinition& roomDef)
{
    if (!ReadString8(fileStream, 32, roomDef.mRoomName))
        return false;

    // resources
    if (!ReadArtResource(fileStream, roomDef.mGuiIcon))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mEditorIcon))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mCompleteResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mStraightResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mInsideCornerResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mUnknownResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mOutsideCornerResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mWallResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mCapResource))
        return false;

    if (!ReadArtResource(fileStream, roomDef.mCeilingResource))
        return false;

    unsigned int ceilingHeight;
    READ_FSTREAM_UINT32(fileStream, ceilingHeight);

    unsigned short fillerWord;
    READ_FSTREAM_UINT16(fileStream, fillerWord);

    unsigned short torchIntensity;
    READ_FSTREAM_UINT16(fileStream, torchIntensity);

    if (!ReadRoomFlags(fileStream, roomDef))
        return false;

    unsigned short tooltipStringId;
    READ_FSTREAM_UINT16(fileStream, tooltipStringId);

    unsigned short nameStringId;
    READ_FSTREAM_UINT16(fileStream, nameStringId);
    READ_FSTREAM_UINT16(fileStream, roomDef.mCost);

    unsigned short fightEffectId;
    READ_FSTREAM_UINT16(fileStream, fightEffectId);

    unsigned short generalDescriptionStringId;
    READ_FSTREAM_UINT16(fileStream, generalDescriptionStringId);

    unsigned short strenghtStringId;
    READ_FSTREAM_UINT16(fileStream, strenghtStringId);

    unsigned short torchRadius;
    READ_FSTREAM_UINT16(fileStream, torchRadius);

    unsigned short effects[8];
    for (unsigned short& effectEntry : effects)
    {
        READ_FSTREAM_UINT16(fileStream, effectEntry);
    }
    READ_FSTREAM_UINT8(fileStream, roomDef.mRoomType);

    unsigned char fillerByte;
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, roomDef.mTerrainType);

    unsigned char tileConstruction;
    READ_FSTREAM_UINT8(fileStream, tileConstruction);
    if (!KwdToENUM(tileConstruction, roomDef.mTileConstruction))
        return false;

    unsigned char createdCreatureId;
    READ_FSTREAM_UINT8(fileStream, createdCreatureId);

    unsigned char torchColor[3];
    for (unsigned char& colorComponent : torchColor)
    {
        READ_FSTREAM_UINT8(fileStream, colorComponent);
    }

    for (int& objectid : roomDef.mFloorObjectsIds)
    {
        READ_FSTREAM_UINT8(fileStream, objectid);
    }
    for (int& objectid : roomDef.mWallObjectsIds)
    {
        READ_FSTREAM_UINT8(fileStream, objectid);
    }

    if (!ReadString8(fileStream, 32, roomDef.mSoundCategory))
        return false;

    READ_FSTREAM_UINT8(fileStream, roomDef.mOrderInEditor);
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    if (!ReadArtResource(fileStream, roomDef.mTorchResource))
        return false;

    READ_FSTREAM_UINT8(fileStream, roomDef.mRecommendedSizeX);
    READ_FSTREAM_UINT8(fileStream, roomDef.mRecommendedSizeY);

    short healthGain;
    READ_FSTREAM_UINT16(fileStream, healthGain);

    return true;
}

bool ScenarioLoader::ReadRoomsData(BinaryInputStream* fileStream, int numElements)
{
    mScenarioData.mRoomDefs.resize(numElements + 1);
    mScenarioData.mRoomDefs[0] = {}; // dummy element

    // read definitions
    for (int iroom = 1; iroom < numElements + 1; ++iroom)
    {
        if (!ReadRoomDefinition(fileStream, mScenarioData.mRoomDefs[iroom]))
            return false;

        bool correctId = (mScenarioData.mRoomDefs[iroom].mRoomType == iroom);
        debug_assert(correctId);
    }

    return true;
}

bool ScenarioLoader::ReadCreaturesDefinition(BinaryInputStream* fileStream, const KwdFileHeader& header, CreatureDefinition& creature)
{
    long startoffset = fileStream->GetCursorPosition();
    long elementSize = header.mContentSize / header.mItemsCount;
    debug_assert(elementSize);

    if (!ReadString8(fileStream, 32, creature.mCreatureName))
        return false;

    SKIP_FSTREAM_BYTES(fileStream, 84); // unknown data
    
    // parse primary animations
    for (int ianim = 0; ianim < 36; ++ianim)
    {
        if (!ReadArtResource(fileStream, creature.mAnimationResources[ianim]))
            return false;
    }

    if (!fileStream->SetCursorPosition(startoffset + elementSize))
        return false;

    return true;
}

bool ScenarioLoader::ReadCreaturesData(BinaryInputStream* fileStream, const KwdFileHeader& header)
{
    mScenarioData.mCreatureDefs.resize(header.mItemsCount + 1);
    mScenarioData.mCreatureDefs[0] = {}; // dummy element

    long startoffset = fileStream->GetCursorPosition();
    // read definitions
    for (int icreature = 1; icreature < header.mItemsCount + 1; ++icreature)
    {
        if (!ReadCreaturesDefinition(fileStream, header, mScenarioData.mCreatureDefs[icreature]))
            return false;

//         bool correctId = (mScenarioData.mCreatureDefs[icreature].mCreatureClass == icreature);
//         debug_assert(correctId);
    }
    long endoffset = fileStream->GetCursorPosition();

    debug_assert(endoffset - startoffset == header.mContentSize);
    return true;
}

bool ScenarioLoader::ReadTerrainDefinition(BinaryInputStream* fileStream, TerrainDefinition& terrainDef)
{
    terrainDef = {};

    if (!ReadString8(fileStream, 32, terrainDef.mName))
        return false;

    // complete resource
    if (!ReadArtResource(fileStream, terrainDef.mResourceComplete))
        return false;

    // side resource
    if (!ReadArtResource(fileStream, terrainDef.mResourceSide))
        return false;

    // top resource
    if (!ReadArtResource(fileStream, terrainDef.mResourceTop))
        return false;

    // tagged top resource
    if (!ReadArtResource(fileStream, terrainDef.mResourceTagged))
        return false;

    if (!ReadStringId(fileStream))
        return false;

    unsigned int depthInt;
    READ_FSTREAM_UINT32(fileStream, depthInt);

    if (!Read32bitsFloat(fileStream, terrainDef.mLightHeight))
        return false;

    if (!ReadTerrainFlags(fileStream, terrainDef))
        return false;

    READ_FSTREAM_UINT16(fileStream, terrainDef.mDamage);

    unsigned short filledWord;
    READ_FSTREAM_UINT16(fileStream, filledWord);
    READ_FSTREAM_UINT16(fileStream, filledWord);

    READ_FSTREAM_UINT16(fileStream, terrainDef.mGoldCapacity);
    READ_FSTREAM_UINT16(fileStream, terrainDef.mManaGain);
    READ_FSTREAM_UINT16(fileStream, terrainDef.mManaGainMax);

    unsigned short toolTipStringId;
    READ_FSTREAM_UINT16(fileStream, toolTipStringId);

    unsigned short nameStringId;
    READ_FSTREAM_UINT16(fileStream, nameStringId);

    unsigned short maxHealthEffectId;
    READ_FSTREAM_UINT16(fileStream, maxHealthEffectId);

    unsigned short destroyedEffectId;
    READ_FSTREAM_UINT16(fileStream, destroyedEffectId);

    unsigned short generalDescriptionStringId;
    READ_FSTREAM_UINT16(fileStream, generalDescriptionStringId);

    unsigned short strengthStringId;
    READ_FSTREAM_UINT16(fileStream, strengthStringId);

    unsigned short weaknessStringId;
    READ_FSTREAM_UINT16(fileStream, weaknessStringId);

    SKIP_FSTREAM_BYTES(fileStream, 16 * sizeof(unsigned short)); // unknown

    unsigned char wibbleH;
    READ_FSTREAM_UINT8(fileStream, wibbleH);

    unsigned char leanH[3];
    for (unsigned char& entryLeanH : leanH)
    {
        READ_FSTREAM_UINT8(fileStream, entryLeanH);
    }

    unsigned char wibbleV;
    READ_FSTREAM_UINT8(fileStream, wibbleV);

    unsigned char leanV[3];
    for (unsigned char& entryLeanV : leanV)
    {
        READ_FSTREAM_UINT8(fileStream, entryLeanV);
    }

    READ_FSTREAM_UINT8(fileStream, terrainDef.mTerrainType);
    READ_FSTREAM_UINT16(fileStream, terrainDef.mHealthInitial);
    READ_FSTREAM_UINT8(fileStream, terrainDef.mBecomesTerrainTypeWhenMaxHealth);
    READ_FSTREAM_UINT8(fileStream, terrainDef.mBecomesTerrainTypeWhenDestroyed);

    unsigned char colorComponents[3];
    // terrain color
    READ_FSTREAM_UINT8(fileStream, colorComponents[0]);
    READ_FSTREAM_UINT8(fileStream, colorComponents[1]);
    READ_FSTREAM_UINT8(fileStream, colorComponents[2]);

    terrainDef.mTerrainColor.Setup( // details unknown
        (terrainDef.mTerrainColorR) ? colorComponents[0] : colorComponents[0],
        (terrainDef.mTerrainColorG) ? colorComponents[1] : colorComponents[1],
        (terrainDef.mTerrainColorB) ? colorComponents[2] : colorComponents[2],
        255
    );

    READ_FSTREAM_UINT8(fileStream, terrainDef.mTextureFrames);

    std::string soundCategory;
    if (!ReadString8(fileStream, 32, soundCategory))
        return false;

    READ_FSTREAM_UINT16(fileStream, terrainDef.mHealthMax);

    // ambient color
    READ_FSTREAM_UINT8(fileStream, colorComponents[0]);
    READ_FSTREAM_UINT8(fileStream, colorComponents[1]);
    READ_FSTREAM_UINT8(fileStream, colorComponents[2]);
    terrainDef.mAmbientColor.Setup( // details unknown
        (terrainDef.mAmbientColorR) ? colorComponents[0] : colorComponents[0],
        (terrainDef.mAmbientColorG) ? colorComponents[1] : colorComponents[1],
        (terrainDef.mAmbientColorB) ? colorComponents[2] : colorComponents[2],
        255
    );

    std::string soundCategoryFirstPerson;
    if (!ReadString8(fileStream, 32, soundCategoryFirstPerson))
        return false;

    unsigned int fillerDword;
    READ_FSTREAM_UINT32(fileStream, fillerDword);

    return true;
}

bool ScenarioLoader::ReadTerrainData(BinaryInputStream* fileStream, int numElements)
{
    mScenarioData.mTerrainDefs.resize(numElements + 1);
    mScenarioData.mTerrainDefs[0] = {}; // dummy element

    // read definitions
    for (int ielement = 1; ielement < numElements + 1; ++ielement)
    {
        if (!ReadTerrainDefinition(fileStream, mScenarioData.mTerrainDefs[ielement]))
            return false;

        bool correctId = (mScenarioData.mTerrainDefs[ielement].mTerrainType == ielement);
        debug_assert(correctId);
    }
    return true;
}

bool ScenarioLoader::ReadLevelVariables(BinaryInputStream* fileStream)
{
    unsigned short fillerWord;
    unsigned int fillerDword;

    READ_FSTREAM_UINT16(fileStream, fillerWord); // trigger id
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    SKIP_FSTREAM_BYTES(fileStream, 520); // unknown data

    // read text messages
    std::vector<std::string> levelMessages;
    levelMessages.resize(512);
    for (std::string& messageEntry: levelMessages)
    {
        if (!ReadString(fileStream, 20, messageEntry))
            return false;
    }

    READ_FSTREAM_UINT16(fileStream, fillerWord); // flags

    std::string speechString;
    if (!ReadString8(fileStream, 32, speechString))
        return false;

    unsigned char talismanPieces;
    READ_FSTREAM_UINT8(fileStream, talismanPieces);
    READ_FSTREAM_UINT32(fileStream, fillerDword);
    READ_FSTREAM_UINT32(fileStream, fillerDword);

    unsigned char soundtrack;
    unsigned char textTableId;
    READ_FSTREAM_UINT8(fileStream, soundtrack);
    READ_FSTREAM_UINT8(fileStream, textTableId);
    READ_FSTREAM_UINT16(fileStream, fillerWord); // textTitleId
    READ_FSTREAM_UINT16(fileStream, fillerWord); // textPlotId
    READ_FSTREAM_UINT16(fileStream, fillerWord); // textDebriefId
    READ_FSTREAM_UINT16(fileStream, fillerWord); // textObjectvId
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord); // speclvlIdx

    // unknown data
    SKIP_FSTREAM_BYTES(fileStream, 8 * sizeof(unsigned char));
    SKIP_FSTREAM_BYTES(fileStream, 8 * sizeof(unsigned short));

    // path
    std::string terrainPath;
    if (!ReadString8(fileStream, 32, terrainPath))
        return false;

    unsigned char oneShotHornyLev;
    unsigned char fillerByte;
    READ_FSTREAM_UINT8(fileStream, oneShotHornyLev);
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);
    READ_FSTREAM_UINT8(fileStream, fillerByte);

    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);
    READ_FSTREAM_UINT16(fileStream, fillerWord);

    std::string heroName;
    if (!ReadString(fileStream, 32, heroName))
        return false;

    return true;
}

bool ScenarioLoader::ReadMapInfo(BinaryInputStream* fileStream)
{
    if (!fileStream->SetCursorPosition(20))
        return false;

    // additional header data

    unsigned short pathCount;
    unsigned short unknownCount;
    unsigned int fillerDword;

    READ_FSTREAM_UINT16(fileStream, pathCount);
    READ_FSTREAM_UINT16(fileStream, unknownCount);
    READ_FSTREAM_UINT32(fileStream, fillerDword);

    // timestamp 1
    if (!ReadTimestamp(fileStream))
        return false;

    // timestamp 2
    if (!ReadTimestamp(fileStream))
        return false;

    READ_FSTREAM_UINT32(fileStream, fillerDword);
    READ_FSTREAM_UINT32(fileStream, fillerDword);

    // property data
    if (!ReadString(fileStream, 64, mScenarioData.mLevelName))
        return false;

    if (!ReadString(fileStream, 1024, mScenarioData.mLevelDescription))
        return false;

    if (!ReadString(fileStream, 64, mScenarioData.mLevelAuthor))
        return false;

    if (!ReadString(fileStream, 64, mScenarioData.mLevelEmail))
        return false;

    if (!ReadString(fileStream, 1024, mScenarioData.mLevelInformation))
        return false;

    // variables
    if (!ReadLevelVariables(fileStream))
        return false;

    SKIP_FSTREAM_BYTES(fileStream, 8); // unknown data

    // read paths
    mPaths.resize(pathCount);
    for (LevelDataFilePath& pathEntry: mPaths)
    {
        READ_FSTREAM_UINT32(fileStream, pathEntry.mId);
        READ_FSTREAM_UINT32(fileStream, fillerDword);

        if (!ReadString8(fileStream, 64, pathEntry.mFilePath))
            return false;
    }
    // unknown data
    SKIP_FSTREAM_BYTES(fileStream, unknownCount * sizeof(unsigned short));
    return !!fileStream;
}

bool ScenarioLoader::ReadDataFile(BinaryInputStream* fileStream, eLevelDataFile dataTypeId)
{
    KwdFileHeader headerData;
    if (dataTypeId == DKLD_GLOBALS)
    {
        // skip global info
        return true;
    }

    // read header
    unsigned int headerId;
    READ_FSTREAM_UINT32(fileStream, headerId);
    if (headerId != dataTypeId) // data type id mistmatch
        return false;

    unsigned int byteSize = 0;
    unsigned int checkOne = 0;
    READ_FSTREAM_UINT32(fileStream, byteSize);
    READ_FSTREAM_UINT32(fileStream, headerData.mFileSize);
    READ_FSTREAM_UINT32(fileStream, checkOne);
    READ_FSTREAM_UINT32(fileStream, headerData.mHeaderEndOffset);

    debug_assert(byteSize == sizeof(unsigned int));

    long startoffset = fileStream->GetCursorPosition();
    switch (dataTypeId)
    {
        case eLevelDataFile::DKLD_MAP:
        {
            READ_FSTREAM_UINT32(fileStream, mScenarioData.mLevelDimensionX);
            READ_FSTREAM_UINT32(fileStream, mScenarioData.mLevelDimensionY);           
        }
        break;
        case eLevelDataFile::DKLD_TRIGGERS:
        {
            SKIP_FSTREAM_BYTES(fileStream, 4); //itemcount 1
            SKIP_FSTREAM_BYTES(fileStream, 4); //itemcount 2
            SKIP_FSTREAM_BYTES(fileStream, 4); //unknown

            if (!ReadTimestamp(fileStream)) // created
                return false;
            if (!ReadTimestamp(fileStream)) // modified
                return false;
        }
        break;
        case eLevelDataFile::DKLD_LEVEL:
        {
            READ_FSTREAM_UINT16(fileStream, headerData.mItemsCount);
            SKIP_FSTREAM_BYTES(fileStream, 2); // height
            SKIP_FSTREAM_BYTES(fileStream, 4); // unknown

            if (!ReadTimestamp(fileStream)) // created
                return false;
            if (!ReadTimestamp(fileStream)) // modified
                return false;
        }
        break;
        default:
        {
            READ_FSTREAM_UINT32(fileStream, headerData.mItemsCount);
            SKIP_FSTREAM_BYTES(fileStream, 4); // unknown

            if (!ReadTimestamp(fileStream)) // created
                return false;
            if (!ReadTimestamp(fileStream)) // modified
                return false;
        }
        break;
    }

    unsigned int checkTwo;
    READ_FSTREAM_UINT32(fileStream, checkTwo);
    READ_FSTREAM_UINT32(fileStream, headerData.mContentSize);

    // read body
    switch (dataTypeId)
    {
        case eLevelDataFile::DKLD_GLOBALS:
        break;
        case eLevelDataFile::DKLD_MAP:
        {
            if (!ReadMapData(fileStream))
                return false;
        }
        break;
        case eLevelDataFile::DKLD_TERRAIN:
        {
            if (!ReadTerrainData(fileStream, headerData.mItemsCount))
                return false;
        }
        break;
        case eLevelDataFile::DKLD_ROOMS:
        {
            if (!ReadRoomsData(fileStream, headerData.mItemsCount))
                return false;
        }
        break;
        case eLevelDataFile::DKLD_TRAPS:
        break;
        case eLevelDataFile::DKLD_DOORS:
        break;
        case eLevelDataFile::DKLD_KEEPER_SPELLS:
        break;
        case eLevelDataFile::DKLD_CREATURE_SPELLS:
        break;
        case eLevelDataFile::DKLD_CREATURES:
        {
            if (!ReadCreaturesData(fileStream, headerData))
                return false;
        }
        break;
        case eLevelDataFile::DKLD_PLAYERS:
        {
            if (!ReadPlayersData(fileStream, headerData.mItemsCount))
                return false;
        }
        break;
        case eLevelDataFile::DKLD_THINGS:
        break;
        case eLevelDataFile::DKLD_TRIGGERS:
        break;
        case eLevelDataFile::DKLD_LEVEL:
        break;
        case eLevelDataFile::DKLD_VARIABLES:
        break;
        case eLevelDataFile::DKLD_OBJECTS:
        {
            if (!ReadObjectsData(fileStream, headerData.mItemsCount))
                return false;
        }
        break;
        case eLevelDataFile::DKLD_EFFECT_ELEMENTS:
        break;
        case eLevelDataFile::DKLD_SHOTS:
        break;
        case eLevelDataFile::DKLD_EFFECTS:
        break;
    }

    return true;
}

bool ScenarioLoader::ScanTerrainTypes()
{
    // all definitions are loaded at this point, so we should map rooms to terrain types
    bool roomAndTerrainDefinitionsNotNull = !(mScenarioData.mRoomDefs.empty() || mScenarioData.mTerrainDefs.empty());
    if (!roomAndTerrainDefinitionsNotNull)
        return false;

    mScenarioData.mRoomByTerrainType.resize(mScenarioData.mTerrainDefs.size(), RoomType_Null);
        
    // explore each room definition
    for (RoomDefinition& roomDefinition : mScenarioData.mRoomDefs)
    {
        // null definition is being skipped
        if (roomDefinition.mRoomType == RoomType_Null)
            continue;

        // bind identifier
        mScenarioData.mRoomByTerrainType[roomDefinition.mTerrainType] = roomDefinition.mRoomType;
    }

    // find special terrain types
    for (TerrainDefinition& terrainDefinition : mScenarioData.mTerrainDefs)
    {
        // null definition is being skipped
        if (terrainDefinition.mTerrainType == TerrainType_Null)
            continue;

        if (terrainDefinition.mIsLava)
        {
            mScenarioData.mLavaTerrainType = terrainDefinition.mTerrainType;
        }

        if (terrainDefinition.mIsWater)
        {
            mScenarioData.mWaterTerrainType = terrainDefinition.mTerrainType;
        }

        if (terrainDefinition.mPlayerColouredPath && (mScenarioData.mPlayerColouredPathTerrainType == TerrainType_Null))
        {
            if (mScenarioData.mRoomByTerrainType[terrainDefinition.mTerrainType] == RoomType_Null)
            {
                mScenarioData.mPlayerColouredPathTerrainType = terrainDefinition.mTerrainType;
            }
        }

        if (terrainDefinition.mPlayerColouredWall)
        {
            mScenarioData.mPlayerColouredWallTerrainType = terrainDefinition.mTerrainType;
        }
    }

    return true;
}

void ScenarioLoader::FixTerrainResources()
{
    // hero lair complete resource is missed, seems it originally hardcoded in game exe
    const int HeroLairTerrainIdentifier = 35;

    debug_assert(HeroLairTerrainIdentifier < mScenarioData.mTerrainDefs.size());
    TerrainDefinition& terrainDef = mScenarioData.mTerrainDefs[HeroLairTerrainIdentifier];
    if (!terrainDef.mResourceComplete.IsDefined())
    {
        terrainDef.mResourceComplete.mResourceType = eArtResource_TerrainMesh;
        terrainDef.mResourceComplete.mResourceName = "hero_outpost_floor";
    }
    else
    {
        debug_assert(false);
    }

    // does not use player colors
    terrainDef.mPlayerColouredPath = false;
}

bool ScenarioLoader::LoadScenarioData(const std::string& scenario)
{
    mScenarioData.Clear();

    std::string scenarioName = scenario;
    cxx::path_remove_extension(scenarioName);

    std::string scenarioFileName = scenario;
    cxx::path_set_extension(scenarioFileName, ".kwd");

    // open file stream
    BinaryInputStream* fileStream = gFileSystem.OpenDataFile(scenarioFileName);
    if (fileStream == nullptr)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open scenario file '%s'", scenarioName.c_str());
        return false;
    }

    if (!ReadMapInfo(fileStream))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Error reading scenario data '%s'", scenarioName.c_str());
        return false;
    }

    gFileSystem.CloseFileStream(fileStream);

    // override paths
    mPaths.clear();

    // globals
    mPaths.emplace_back(eLevelDataFile::DKLD_TERRAIN, "../Terrain.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_OBJECTS, "../Objects.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_ROOMS, "../Rooms.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_CREATURES, "../Creatures.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_CREATURE_SPELLS, "../CreatureSpells.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_TRAPS, "../Traps.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_DOORS, "../Doors.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_SHOTS, "../Shots.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_KEEPER_SPELLS, "../KeeperSpells.kwd");
    mPaths.emplace_back(eLevelDataFile::DKLD_VARIABLES, "../GlobalVariables.kwd");

    // level data
    mPaths.emplace_back(eLevelDataFile::DKLD_PLAYERS, scenarioName + "Players.kld");
    mPaths.emplace_back(eLevelDataFile::DKLD_MAP, scenarioName + "Map.kld");
    mPaths.emplace_back(eLevelDataFile::DKLD_TRIGGERS, scenarioName + "Triggers.kld");
    mPaths.emplace_back(eLevelDataFile::DKLD_VARIABLES, scenarioName + "Variables.kld");
    mPaths.emplace_back(eLevelDataFile::DKLD_THINGS, scenarioName + "Things.kld");

    // read data from data files
    for (const LevelDataFilePath& pathEntry: mPaths)
    {
        BinaryInputStream* dataFileStream = gFileSystem.OpenDataFile(pathEntry.mFilePath);
        if (dataFileStream == nullptr)
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot locate scenario data file '%s'", pathEntry.mFilePath.c_str());
            continue;   
        }
        if (!ReadDataFile(fileStream, pathEntry.mId))
        {
            gConsole.LogMessage(eLogMessage_Warning, "Error reading scenario data file '%s'", pathEntry.mFilePath.c_str());
            return false;
        }    
        gFileSystem.CloseFileStream(dataFileStream);
    }

    if (!ScanTerrainTypes())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Error exploring scenario terrain types");
        return false;
    }

    FixTerrainResources();
    return true;
}
